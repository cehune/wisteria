//
//  pathtracer.metal
//  wisteria
//
//  Created by celine on 2026-03-31.
//

#include <metal_stdlib>
#include <metal_raytracing>
#include "../common/Types.h"
#include "../../../engine/gpu/SharedTypes.h"
#include "../common/Spectrum.h"
#include "../common/Frame.h"
#include "../common/bxdf/Lambertian.h"
#include "../common/sampler/IndependentSampler.h"
#include "../common/Light.h"

using namespace metal;
using namespace raytracing;

// Constant/gradient environment light — the only emitter for now.
// Flatten to a constant Spectrum(x) for the white-furnace test.
inline Spectrum env_radiance(float3 d) {
    return Spectrum(0.0f);   // enclosed Cornell box: no environment light
}

kernel void raytrace_kernel(
    texture2d<float, access::write>       outTex       [[texture(0)]],
    texture2d<float, access::read_write>  accumTex     [[texture(1)]],
    const device Vertex*                vertices     [[buffer(0)]],
    const device uint*                    indices      [[buffer(1)]],
    constant CameraUniformsPT&            cam          [[buffer(3)]],
    constant uint&                        sampleCount  [[buffer(4)]],
    instance_acceleration_structure       accel        [[buffer(5)]],
    const device InstanceData*            instanceData [[buffer(6)]],
    const device Material*                materials    [[buffer(7)]],
    const device Light*                   lights       [[buffer(8)]],
    constant uint&                        numLights    [[buffer(9)]],
    uint2                                 gid          [[thread_position_in_grid]])
{
    uint W = outTex.get_width();
    uint H = outTex.get_height();

    // per-(pixel, sample) RNG; jitter the primary ray for anti-aliasing
    IndependentSampler rng = make_sampler(gid, sampleCount);
    float2 uv  = (float2(gid) + next_2d(rng)) / float2(W, H);
    float2 ndc = uv * 2.0f - 1.0f;

    float aspect = float(W) / float(H);
    float halfH  = tan(cam.fov * 0.5f);

    ray r;
    r.origin       = cam.origin;
    r.direction    = normalize(cam.forward
                             + cam.right * (ndc.x * aspect * halfH)
                             + cam.up    * (-ndc.y * halfH));
    r.min_distance = 1e-4f;
    r.max_distance = INFINITY;

    // --- unidirectional path tracer: BSDF sampling against a constant-env light ---
    // No NEE / MIS / Russian roulette yet . Albedo is a placeholder
    // until the material buffer lands (next commit).
    const uint MAX_DEPTH = 8;

    Spectrum throughput = Spectrum(1.0f);
    Spectrum L          = Spectrum(0.0f);
    intersector<triangle_data, instancing> isect;

    for (uint depth = 0; depth < MAX_DEPTH; ++depth) {
        auto hit = isect.intersect(r, accel);
        if (hit.type != intersection_type::triangle) {
            L += throughput * env_radiance(r.direction);   // ray escaped to the sky
            break;
        }

        // triangle -> interpolated world shading normal + this instance's material
        InstanceData inst   = instanceData[hit.instance_id];
        Spectrum     albedo = materials[inst.materialID].albedo;
        uint base = inst.indexOffset + hit.primitive_id * 3;
        Vertex vA = vertices[indices[base + 0]];
        Vertex vB = vertices[indices[base + 1]];
        Vertex vC = vertices[indices[base + 2]];
        float2 bc = hit.triangle_barycentric_coord;
        float  wA = 1.0f - bc.x - bc.y, wB = bc.x, wC = bc.y;

        float3   nObj = normalize(wA * vA.normal.xyz + wB * vB.normal.xyz + wC * vC.normal.xyz);
        float4x4 o2w  = inst.transform;
        float3   ns   = normalize(float3x3(o2w[0].xyz, o2w[1].xyz, o2w[2].xyz) * nObj);

        float3 hitP = r.origin + r.direction * hit.distance;
        float3 woW  = -r.direction;

        // emitter hit: only count on primary rays — NEE handles direct lighting at deeper bounces
        if (inst.lightID >= 0 && depth == 0) {
            float3 pA = (o2w * vA.position).xyz;
            float3 pB = (o2w * vB.position).xyz;
            float3 pC = (o2w * vC.position).xyz;
            float3 gN = normalize(cross(pB - pA, pC - pA));
            Light  lt = lights[inst.lightID];
            if (lt.twoSided != 0 || dot(gN, woW) > 0.0f)
                L += throughput * lt.radiance;
        }

        if (dot(ns, woW) < 0.0f) ns = -ns;              // face the viewer (two-sided)

        // shading frame + local outgoing direction
        Frame  frame = Frame::fromNormal(ns);
        float3 wo    = frame.toLocal(woW);

        // --- NEE: sample one light, add direct contribution if unoccluded ---
        if (numLights > 0) {
            uint         li    = min(uint(next_1d(rng) * float(numLights)), numLights - 1u);
            Light        lt    = lights[li];
            InstanceData lInst = instanceData[lt.instanceID];

            LightSample ls = light_sample_Li(lt, lInst, numLights, hitP,
                                             next_1d(rng), next_2d(rng),
                                             vertices, indices);

            if (ls.pdf > 0.0f && !is_black(ls.Li)) {
                ray shadowRay;
                shadowRay.origin       = hitP + ns * 1e-3f;
                shadowRay.direction    = ls.wi;
                shadowRay.min_distance = 1e-4f;
                shadowRay.max_distance = ls.dist - 1e-3f;

                intersector<triangle_data, instancing> shadowIsect;
                shadowIsect.accept_any_intersection(true);
                auto shadowHit = shadowIsect.intersect(shadowRay, accel);

                if (shadowHit.type == intersection_type::none) {
                    float3   wiL  = frame.toLocal(ls.wi);
                    Spectrum f    = lambertian_eval(albedo, wo, wiL);
                    float    cosL = abs(cosTheta(wiL));
                    L += throughput * f * ls.Li * cosL / ls.pdf;
                }
            }
        }

        // sample the BSDF for the next bounce
        BSDFSample bs = lambertian_sample(albedo, wo, next_2d(rng));
        if (bs.pdf <= 0.0f) break;

        // throughput *= f * cos / pdf   ( == albedo for a Lambertian )
        throughput *= bs.f * abs(cosTheta(bs.wi)) / bs.pdf;

        // continuation ray, nudged off the surface to avoid self-intersection
        r.origin       = hitP + ns * 1e-3f;
        r.direction    = frame.toWorld(bs.wi);
        r.min_distance = 1e-4f;
        r.max_distance = INFINITY;
    }

    // cumulative moving average across accumulated samples
    float4 color = float4(L, 1.0f);
    float4 prev  = accumTex.read(gid);
    float4 accum = (prev * float(sampleCount) + color) / float(sampleCount + 1);
    accumTex.write(accum, gid);
    outTex.write(accum, gid);
}

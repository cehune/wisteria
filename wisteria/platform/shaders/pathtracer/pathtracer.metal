//
//  pathtracer.metal
//  wisteria
//
//  Created by celine on 2026-03-31.
//

#include <metal_stdlib>
#include <metal_raytracing>
#include "Types.hpp"

using namespace metal;
using namespace raytracing;

kernel void raytrace_kernel(
    texture2d<float, access::write>       outTex       [[texture(0)]],
    texture2d<float, access::read_write>  accumTex     [[texture(1)]],
    const device VertexIn*                vertices     [[buffer(0)]],
    const device uint*                    indices      [[buffer(1)]],
    constant CameraUniformsPT&            cam          [[buffer(3)]],
    constant uint&                        sampleCount  [[buffer(4)]],
    instance_acceleration_structure       accel        [[buffer(5)]],
    const device InstanceData*            instanceData [[buffer(6)]],
    uint2                                 gid          [[thread_position_in_grid]])
{
    uint W = outTex.get_width();
    uint H = outTex.get_height();

    // pixel -> NDC [-1, 1]
    float2 uv  = float2(gid) / float2(W, H);
    float2 ndc = uv * 2.0 - 1.0;

    float aspect = float(W) / float(H);
    float halfH  = tan(cam.fov * 0.5);

    // primary ray through this pixel
    ray r;
    r.origin       = cam.origin;
    r.direction    = normalize(cam.forward
                             + cam.right * (ndc.x * aspect * halfH)
                             + cam.up    * (-ndc.y * halfH));
    r.min_distance = 1e-4f;
    r.max_distance = INFINITY;

    // closest-hit traversal of the TLAS (opaque, instanced triangles)
    intersector<triangle_data, instancing> isect;
    auto hit = isect.intersect(r, accel);

    float4 color = float4(0.0, 0.0, 0.0, 1.0);
    if (hit.type == intersection_type::triangle) {
        // instance_id -> this mesh's index range; primitive_id -> its triangle.
        InstanceData inst = instanceData[hit.instance_id];
        uint base = inst.indexOffset + hit.primitive_id * 3;

        VertexIn vA = vertices[indices[base + 0]];
        VertexIn vB = vertices[indices[base + 1]];
        VertexIn vC = vertices[indices[base + 2]];

        float2 bc = hit.triangle_barycentric_coord;
        float  wA = 1.0 - bc.x - bc.y;
        float  wB = bc.x;
        float  wC = bc.y;

        // object-space smooth normal, then to world via the instance transform's
        // upper 3x3 (exact for rigid / uniform scale). The transform is carried in
        // InstanceData — intersection_result doesn't expose it under these tags.
        float3   nObj   = normalize(wA * vA.normal.xyz + wB * vB.normal.xyz + wC * vC.normal.xyz);
        float4x4 o2w    = inst.transform;
        float3   nWorld = normalize(float3x3(o2w[0].xyz, o2w[1].xyz, o2w[2].xyz) * nObj);

        color = float4(nWorld * 0.5 + 0.5, 1.0);
    }

    // cumulative moving average across accumulated samples
    float4 prev  = accumTex.read(gid);
    float4 accum = (prev * float(sampleCount) + color) / float(sampleCount + 1);
    accumTex.write(accum, gid);
    outTex.write(accum, gid);
}

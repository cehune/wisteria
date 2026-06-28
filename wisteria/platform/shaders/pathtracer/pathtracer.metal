//
//  pathtracer.metal
//  wisteria
//
//  Created by celine on 2026-03-31.
//

#include <metal_stdlib>
#include "Types.hpp"

using namespace metal;

static HitResult moller_trumbore(float3 a, float3 b, float3 c, Ray ray, float epsilon) {
    // this is moller trumbore assuming that the its CCW
    float3 edge1 = b - a;
    float3 edge2 = c - a;
        
    // now we do cramers rule with determinants to solve for t, u, v simulatenously
    // treat A = [-D, edge1, edge2], the Ai with the solution T = (O - a) along the diagonal of A
    // with the solution Xi = det(Ai) / det(A)

    // parallel or front facing test
    float3 P = cross(ray.direction, edge2);
    float det_A = dot(P, edge1);
    
    if (abs(det_A) < epsilon) return { false, 0, 0, 0 };
    
    float3 T = ray.origin - a;
    float3 Q = cross(T, edge1);
    float u = dot(P, T) / det_A;
    float v = dot(Q, ray.direction) / det_A;
    
    // barycentric coord checks
    if (u < 0.0 || u > 1.0) return { false, 0, 0, 0 };
    if (v < 0.0 || u + v > 1.0) return { false, 0, 0, 0 };
    
    float t = dot(Q, edge2) / det_A;
    
    if (t > epsilon) {
        return {true, t, float3(ray.origin + ray.direction * t), float2(u, v)};
    }
    return {false, 0, 0, 0 };
}


kernel void raytrace_kernel(
    texture2d<float, access::write>  outTex   [[texture(0)]],
    texture2d<float, access::read_write> accumTex [[texture(1)]],
    const device VertexIn*           vertices [[buffer(0)]],
    const device uint*               indices  [[buffer(1)]],
    constant uint&                   numTri   [[buffer(2)]],
    constant CameraUniformsPT&       cam      [[buffer(3)]],
    constant uint& sampleCount                    [[buffer(4)]],
    uint2                            gid      [[thread_position_in_grid]])
{
    uint W = outTex.get_width();
    uint H = outTex.get_height();
    // clip space 0, 1
    float2 uv = float2(gid) / float2(W, H);
    // NDC -1, 1
    float2 ndc = uv * 2.0 - 1.0;
    
    Ray ray;
    ray.origin = cam.origin;

    float aspect = float(W) / float(H);
    float halfH = tan(cam.fov * 0.5);
    ray.direction = normalize(
        cam.forward
        + cam.right * (ndc.x * aspect * halfH)
        + cam.up    * (-ndc.y * halfH)
    );
    
    HitResult closest;
    closest.hit = false;
    closest.t   = 1e9;

    float4 color = float4(0.0, 0.0, 0.0, 1);
    for (uint i = 0; i < numTri; i++) {
        VertexIn vA = vertices[indices[i * 3]];
        VertexIn vB = vertices[indices[i * 3 + 1]];
        VertexIn vC = vertices[indices[i * 3 + 2]];

        HitResult hit = moller_trumbore(vA.position.xyz, vB.position.xyz, vC.position.xyz, ray, 1e-6);
        if (hit.hit && hit.t < closest.t) {
            closest = hit;
            float wA = 1.0 - hit.bary.x - hit.bary.y;
            float wB = hit.bary.x;
            float wC = hit.bary.y;

            float3 normal = normalize(wA * vA.normal.xyz +
                                          wB * vB.normal.xyz +
                                          wC * vC.normal.xyz);
            color = float4(normal * 0.5 + 0.5, 1.0);
//            color = float4(wA * vA.color.xyz
//                         + hit.bary.x * vB.color.xyz
//                         + hit.bary.y * vC.color.xyz, 1.0);
        }
    }
    float4 prev = accumTex.read(gid);
    float4 accum = (prev * float(sampleCount) + color) / float(sampleCount + 1);
    accumTex.write(accum, gid);

    // write display output
    outTex.write(accum, gid);
}


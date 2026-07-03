//
//  Light.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include <metal_stdlib>
#include "../../../engine/gpu/SharedTypes.h"
#include "Spectrum.h"

using namespace metal;

/*
NEE + MIS logic
only light sample is actually necessary for NEE
But this means that without a MIS PDF, you can't do MIS.

In the pure NEE case you only use NEE for illumination, you can't use the 
information even if a ray BSDF's into an emitter.
*/

struct LightSample {
    float3   wi;    // direction from refP toward light (normalized)
    Spectrum Li;    // radiance arriving at refP
    float    pdf;   // solid-angle pdf
    float    dist;  // distance to sampled point (shadow ray tmax)
};

// Light and LightType come from SharedTypes.h (shared CPU/GPU ABI).

inline float3 light_fetch_world_pos(device const Vertex* verts,
                                    device const uint*   indices,
                                    uint                 startingIndex,
                                    uint                 vertexNumber,
                                    float4x4             transform)
{
    return (transform * verts[indices[startingIndex + vertexNumber]].position).xyz;
}

/*
Private helper for area light sampling. Given a light and its instance data, 
sample a point on the light's surface and return the direction, radiance, pdf, and 
distance to the light from a given surface point. 

Assumes an area light if you're using it. Aka at least one face on a mesh.
*/
inline LightSample light_sample_area(Light light, InstanceData inst, 
                              float3 surfacePoint,
                              float  triangleSelection, // [0, 1) from next_1d()
                              float2 baryCentricCDF,    // [0, 1)^2 from next_2d()
                              device const Vertex* vertices,
                              device const uint*   indices)
{
    LightSample sample;
    sample.wi   = 0.0f;
    sample.Li   = 0.0f;
    sample.pdf  = 0.0f;
    sample.dist = 0.0f;

    uint numTriangles = inst.indexCount / 3;
    uint j    = min(uint(triangleSelection * numTriangles), numTriangles - 1u);
    uint base = inst.indexOffset + j * 3;

    float3 A = light_fetch_world_pos(vertices, indices, base, 0, inst.transform);
    float3 B = light_fetch_world_pos(vertices, indices, base, 1, inst.transform);
    float3 C = light_fetch_world_pos(vertices, indices, base, 2, inst.transform);

    float3 normal = cross(B-A, C-A);
    // ||a cross b|| = ||a|| ||b|| sinθ = area of parallelogram, then divide by 2 for triangle
    float triangleArea = length(normal) * 0.5f; 
    normal = normalize(normal);
    
    // use sqrt warp with cdfs
    // maps two numbers of uniform [0,1) to a uniform u1 + u2 <= 1
    float u1 = sqrt(baryCentricCDF.x);
    float u2 = u1 * baryCentricCDF.y;
    float3 lightPoint  = (1.0f - u1 - u2) * A + u1 * B + u2 * C;

    float3 toLight = lightPoint - surfacePoint;
    float  dist    = length(toLight);
    float3 wi      = toLight / dist;

    // solid-angle pdf: p_A = 1/(N*triArea) TODO: use area weighted later
    float cosLight = abs(dot(wi, normal));
    float pdf      = (dist * dist) / (float(numTriangles) * triangleArea * max(cosLight, 1e-4f));

    bool facing = (light.twoSided != 0u) || (dot(-wi, normal) > 0.0f);

    sample.wi   = wi;
    sample.Li   = facing ? light.radiance : Spectrum(0.0f);
    sample.pdf  = pdf;
    sample.dist = dist;

    return sample;
}

// NEE. Caller looks up instBuffer[L.instanceID] and passes it in.
// Returns a LightSample whose pdf already accounts for uniform light selection (1/numLights).
inline LightSample light_sample_Li(Light L, InstanceData inst, uint numLights,
                            float3 surfacePoint,
                            float  triangleSelection,
                            float2 barycentricCDF,
                            device const Vertex* vertices,
                            device const uint*   indices)
{
    LightSample s;
    switch (L.type) {
        case LIGHT_AREA:
            s = light_sample_area(L, inst, surfacePoint, triangleSelection, barycentricCDF, vertices, indices);
            break;
        case LIGHT_POINT: {
            float3 toLight = L.posOrDir - surfacePoint;
            float  dist    = length(toLight);
            s = LightSample{toLight / dist, L.radiance, 1.0f, dist};
            break;
        }
        case LIGHT_DIRECTIONAL:
            s = LightSample{-L.posOrDir, L.radiance, 1.0f, INFINITY};
            break;
        default:
            return LightSample{0.0f, 0.0f, 0.0f, 0.0f};
    }
    s.pdf /= float(numLights);
    return s;
}

inline float light_pdf_Li(Light L, float3 surfacePoint, float3 wi)
{
    // TODO: need for MIS
    return 0.0f;
}

//
//  Light.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//
//  Platform seam for light sampling: the ONLY light code that touches the GPU mega
//  buffers. It fetches area-light triangles (device) and dispatches to the pure
//  engine samplers in engine/shading/lights/LightSampling.h. Everything with real
//  math lives there and is host-tested; this file is just marshalling.
//

#pragma once
#include <metal_stdlib>
#include "../../../engine/shading/common/SharedTypes.h"
#include "../../../engine/shading/lights/LightSampling.h"

using namespace metal;

// Fetch world-space triangle j of an area-light instance from the mega buffers.
inline void light_fetch_triangle(InstanceData inst, uint j,
                                 device const Vertex* verts, device const uint* indices,
                                 thread float3& A, thread float3& B, thread float3& C)
{
    uint base = inst.indexOffset + j * 3u;
    A = (inst.transform * verts[indices[base + 0]].position).xyz;
    B = (inst.transform * verts[indices[base + 1]].position).xyz;
    C = (inst.transform * verts[indices[base + 2]].position).xyz;
}

// NEE dispatcher. For an area light: uniformly pick a triangle, fetch it, hand off to
// the pure sampler. For delta lights: straight to the pure sampler. Then apply the
// 1/numLights uniform light-selection factor. Caller looks up instBuffer[L.instanceID]
// and passes it in.
inline LightSample light_sample_Li(Light L, InstanceData inst, uint numLights,
                                   float3 refP, float triSel, float2 bary,
                                   device const Vertex* vertices, device const uint* indices)
{
    LightSample s;
    switch (L.type) {
        case LIGHT_AREA: {
            uint   numTri = inst.indexCount / 3u;
            uint   j      = min(uint(triSel * float(numTri)), numTri - 1u);
            float3 A, B, C;
            light_fetch_triangle(inst, j, vertices, indices, A, B, C);
            s = light_sample_area(A, B, C, L.radiance, L.twoSided != 0u, refP, bary, numTri);
            break;
        }
        case LIGHT_POINT:
            s = light_sample_point(L.posOrDir, L.radiance, refP);
            break;
        case LIGHT_DIRECTIONAL:
            s = light_sample_directional(L.posOrDir, L.radiance);
            break;
        default:
            s = LightSample{float3(0.0f), Spectrum(0.0f), 0.0f, 0.0f};
    }
    s.pdf /= float(numLights);
    return s;
}

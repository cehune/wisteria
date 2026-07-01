//
//  Types.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include <metal_stdlib>
#include "../SharedTypes.h"   // InstanceData + Material (shared CPU/GPU ABI)

using namespace metal;

// Vertex (the mega-VB element) now comes from ../SharedTypes.h. VertexOut is the
// vertex-stage output — GPU-only ([[position]]), so it stays here.
struct VertexOut {
    float4 position [[position]];
    float4 normal;
    float2 uv;
    float4 tangent;
    float4 color;
};

// InstanceData + Material now come from ../SharedTypes.h — one definition,
// compiled by both C++ and Metal. Only VertexOut (above) stays GPU-only.

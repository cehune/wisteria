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

struct VertexIn {
    float4 position;
    float4 normal;
    float2 uv;
    float4 tangent;
    float4 color;
};

struct VertexOut {
    float4 position [[position]];
    float4 normal;
    float2 uv;
    float4 tangent;
    float4 color;
};

struct CameraUniforms {
    float4x4 viewProjection;
};

struct CameraUniformsPT {
    float3 origin;
    float3 forward;
    float3 up;
    float3 right;
    float fov;
};

// InstanceData + Material now come from ../SharedTypes.h — one definition,
// compiled by both C++ and Metal. VertexIn / VertexOut / CameraUniforms* above
// stay GPU-only for now.

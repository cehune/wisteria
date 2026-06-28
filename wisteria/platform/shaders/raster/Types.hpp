//
//  Types.h
//  wisteria
//
//  Created by celine on 2026-04-01.
//

// We need to just replicate the stuff we have in engine folder because metal shaders need to use metal types

#pragma once
#include <metal_stdlib>

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

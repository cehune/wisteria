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

struct Ray {
    float3 origin;
    float3 direction;
};

struct HitResult {
    bool hit;
    float t;
    float3 position;
    float2 bary;
};

struct CameraUniformsPT {
    float3 origin;
    float3 forward;
    float3 up;
    float3 right;
    float fov;
};

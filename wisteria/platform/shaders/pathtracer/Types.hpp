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

struct CameraUniformsPT {
    float3 origin;
    float3 forward;
    float3 up;
    float3 right;
    float fov;
};

// Per-instance payload the kernel reads by intersection instance_id, to map a
// hit back to its mesh's index range (+ material). CPU mirror lives in
// AccelStructures.hpp — keep the two in sync by hand.
struct InstanceData {
    uint indexOffset;
    uint materialID;
    float4x4 transform;   // object -> world; upper 3x3 used for normals
};

// GPU mirror of the CPU Material (engine/scene/Material.hpp) — keep in sync.
struct Material {
    uint   type;      // 0 = Lambertian
    float3 albedo;
};

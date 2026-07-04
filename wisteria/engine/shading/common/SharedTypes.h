//
//  SharedTypes.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//
// The CPU<->GPU ABI. Compiled by BOTH C++ and Metal, so structs that live in a
// GPU buffer have exactly ONE definition and cannot drift. Anything that uses
// MSL-only constructs (thread, [[attributes]], device functions) does NOT belong
// here — that stays in platform/shaders (GPU-only).
//
// Scalar/vector types come from the shared `wst` vocabulary (Common.h).
//

#pragma once
#include "Common.h"

// Per-instance shading payload: AccelStructures builds this buffer; the kernel
// reads it by intersection instance_id to recover geometry range + material + light.
struct InstanceData {
    wst::uint_t   indexOffset;
    wst::uint_t   materialID;
    wst::int_t    lightID;      // -1 = not an emitter
    wst::uint_t   indexCount;   // # indices in this mesh's slice (triangles = indexCount / 3)
    wst::float4x4 transform;    // object -> world; upper 3x3 used for normals
}; // sizeof = 80

enum MaterialType : wst::uint_t {
    MATERIAL_LAMBERTIAN = 0,
    MATERIAL_CONDUCTOR  = 1,
};

struct Material {
    wst::uint_t type;          // MaterialType
    wst::float3 albedo;        // Lambertian: diffuse reflectance;  Conductor: F0 (specular colour)
    float       roughness;     // Conductor: perceptual roughness (alpha = roughness^2)
};

struct CameraUniformsPT {
    wst::float3 origin;
    wst::float3 forward;
    wst::float3 up;
    wst::float3 right;
    float       fov;      // plain float is identical in C++ and MSL — no shim
};

// We don't actually need the model matrix, thats stored per mesh
struct CameraUniformsRaster {
    wst::float4x4 viewProjection;
};

// Mega-VB element. The BLAS vertex stride (sizeof) and the kernel's read of this
// struct must agree — exactly why it's shared. (The float2 uv leaves 8 bytes of
// hidden padding before tangent, which is how a hand-mirror would silently drift.)
struct Vertex {
    wst::float4 position;
    wst::float4 normal;
    wst::float2 uv;
    wst::float4 tangent;
    wst::float4 color;
};

enum LightType : wst::uint_t {
    LIGHT_AREA = 0,
    LIGHT_ENVIRONMENT = 1,
    LIGHT_POINT = 2,
    LIGHT_DIRECTIONAL = 3,
};

struct Light {
    wst::float3 radiance;    // Le (area/env) or intensity (point/dir)
    wst::uint_t type;        // LightType
    wst::uint_t instanceID;  // AREA: emissive geometry this light owns
    wst::uint_t twoSided;    // AREA: 0 = emit only along +geometric-normal face
    wst::uint_t flags;       // isDelta, isInfinite
    wst::float3 posOrDir;    // POINT: world position  DIRECTIONAL: direction  ENV: unused
}; // sizeof = 48

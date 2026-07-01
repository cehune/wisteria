//
//  SharedTypes.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//
// The CPU<->GPU ABI. Compiled by BOTH C++ and Metal, so structs that live in a
// GPU buffer have exactly ONE definition and cannot drift. Anything that uses
// MSL-only constructs (thread, [[attributes]], device functions) does NOT belong
// here — that stays in common/ (GPU-only). Test before adding a struct: could a
// plain .cpp #include this and compile? If not, it's the wrong header.

#pragma once


#ifdef __METAL_VERSION__
    #include <metal_stdlib>
    using namespace metal;   // matrix/vector types live in metal::; C++ side never sees this
    #define WST_UINT     uint
    #define WST_FLOAT3   float3
    #define WST_FLOAT4X4 float4x4
#else
    #include <simd/simd.h>
    #include <cstdint>
    #define WST_UINT     uint32_t
    #define WST_FLOAT3   simd_float3
    #define WST_FLOAT4X4 simd_float4x4
#endif

// Per-instance shading payload: AccelStructures builds this buffer; the kernel
// reads it by intersection instance_id to recover geometry range + material.
struct InstanceData {
    WST_UINT     indexOffset;
    WST_UINT     materialID;
    WST_FLOAT4X4 transform;   // object -> world; upper 3x3 used for normals
};

struct Material {
    WST_UINT   type;          // 0 = Lambertian
    WST_FLOAT3 albedo;
};

#undef WST_UINT
#undef WST_FLOAT3
#undef WST_FLOAT4X4

//
//  SharedTypes.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//
// The CPU<->GPU ABI. Compiled by BOTH C++ and Metal, so structs that live in a
// GPU buffer have exactly ONE definition and cannot drift. Anything that uses
// MSL-only constructs (thread, [[attributes]], device functions) does NOT belong
// here that stays in common/ (GPU-only). 
//

#pragma once


#ifdef __METAL_VERSION__
    #include <metal_stdlib>
    using namespace metal;   // matrix/vector types live in metal::; C++ side never sees this
    #define WST_INT      int
    #define WST_UINT     uint
    #define WST_FLOAT2   float2
    #define WST_FLOAT3   float3
    #define WST_FLOAT4   float4
    #define WST_FLOAT4X4 float4x4
#else
    #include <simd/simd.h>
    #include <cstdint>
    #define WST_INT     int32_t
    #define WST_UINT     uint32_t
    #define WST_FLOAT2   simd_float2
    #define WST_FLOAT3   simd_float3
    #define WST_FLOAT4   simd_float4
    #define WST_FLOAT4X4 simd_float4x4
#endif

// Per-instance shading payload: AccelStructures builds this buffer; the kernel
// reads it by intersection instance_id to recover geometry range + material + light.
struct InstanceData {
    WST_UINT     indexOffset;
    WST_UINT     materialID;
    WST_INT      lightID;      // -1 = not an emitter
    WST_UINT     indexCount;   // # indices in this mesh's slice (triangles = indexCount / 3)
    WST_FLOAT4X4 transform;    // object -> world; upper 3x3 used for normals
}; // sizeof = 80

struct Material {
    WST_UINT   type;          // 0 = Lambertian
    WST_FLOAT3 albedo;
};

struct CameraUniformsPT {
    WST_FLOAT3 origin;
    WST_FLOAT3 forward;
    WST_FLOAT3 up;
    WST_FLOAT3 right;
    float      fov;      // plain float is identical in C++ and MSL — no shim
};

// We don't actually need the model matrix, thats stored per mesh
struct CameraUniformsRaster {
    WST_FLOAT4X4 viewProjection;
};

// Mega-VB element. The BLAS vertex stride (sizeof) and the kernel's read of this
// struct must agree — exactly why it's shared. (The float2 uv leaves 8 bytes of
// hidden padding before tangent, which is how a hand-mirror would silently drift.)
struct Vertex {
    WST_FLOAT4 position;
    WST_FLOAT4 normal;
    WST_FLOAT2 uv;
    WST_FLOAT4 tangent;
    WST_FLOAT4 color;
};

enum LightType : WST_UINT {
    LIGHT_AREA = 0,
    LIGHT_ENVIRONMENT = 1,
    LIGHT_POINT = 2,
    LIGHT_DIRECTIONAL = 3,
};

struct Light {
    WST_FLOAT3 radiance;    // Le (area/env) or intensity (point/dir)
    WST_UINT   type;        // LightType
    WST_UINT   instanceID;  // AREA: emissive geometry this light owns
    WST_UINT   twoSided;    // AREA: 0 = emit only along +geometric-normal face
    WST_UINT   flags;       // isDelta, isInfinite
    WST_FLOAT3 posOrDir;    // POINT: world position  DIRECTIONAL: direction  ENV: unused
}; // sizeof = 48


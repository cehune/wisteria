//
//  SharedTypes.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//
// The CPU<->GPU ABI. Compiled by BOTH C++ and Metal, so structs that live in a
// GPU buffer have exactly ONE definition and cannot drift. Anything that uses
// MSL-only constructs (thread, [[attributes]], device functions) does NOT belong
// here. that stays in common/ (GPU-only).
//

#pragma once

#ifdef __METAL_VERSION__
    #include <metal_stdlib>
    using namespace metal;   // shaders including this may use metal vector types unqualified
    namespace wst {
        using int_t    = int;
        using uint_t   = uint;
        using float2   = metal::float2;
        using float3   = metal::float3;
        using float4   = metal::float4;
        using float4x4 = metal::float4x4;
    }
#else
    #include <simd/simd.h>
    #include <cstdint>
    namespace wst {
        using int_t    = int32_t;
        using uint_t   = uint32_t;
        using float2   = simd_float2;
        using float3   = simd_float3;
        using float4   = simd_float4;
        using float4x4 = simd_float4x4;
    }
#endif

// Per-instance shading payload: AccelStructures builds this buffer; the kernel
// reads it by intersection instance_id to recover geometry range + material + light.
struct InstanceData {
    wst::uint_t   indexOffset;
    wst::uint_t   materialID;
    wst::int_t    lightID;      // -1 = not an emitter
    wst::uint_t   indexCount;   // # indices in this mesh's slice (triangles = indexCount / 3)
    wst::float4x4 transform;    // object -> world; upper 3x3 used for normals
}; // sizeof = 80

struct Material {
    wst::uint_t type;          // 0 = Lambertian
    wst::float3 albedo;
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

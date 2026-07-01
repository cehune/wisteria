//
//  Material.hpp
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include <simd/simd.h>
#include <cstdint>

enum MaterialType : uint32_t { MATERIAL_LAMBERTIAN = 0 };

// CPU mirror of the GPU Material in platform/shaders/pathtracer/Types.hpp.
// Keep the two layouts in sync by hand.
// uint32 is fine, metal pads anyways so no diff between 32 and 8
struct Material {
    uint32_t    type;     // MaterialType
    simd_float3 albedo;
};

// all thats needed for diffuse lambertian: type, albedo

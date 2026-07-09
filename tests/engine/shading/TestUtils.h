//
//  TestUtils.h
//  wisteria
//
//  Created by celine on 2026-07-08.
//

#pragma once
#include <simd/simd.h>
#include <cmath>

static inline simd_float3 f3(float x, float y, float z) { return simd_make_float3(x, y, z); }
static inline simd_float2 f2(float x, float y)          { return simd_make_float2(x, y); }

// Uniform direction on the upper hemisphere (pdf = 1/2pi in solid angle).
static inline simd_float3 uniformHemisphere(float u1, float u2) {
    float z   = u1;
    float r   = sqrtf(fmaxf(0.0f, 1.0f - z * z));
    float phi = 2.0f * (float)M_PI * u2;
    return f3(r * cosf(phi), r * sinf(phi), z);
}

//
//  Common.h
//  wisteria
//
//  Cross-compile math vocabulary shared by CPU (simd) and Metal (metal_stdlib).
//
//  Scoped-namespace injection: the backend namespace is pulled INTO `wst` rather
//  than global scope, so `float3`, `dot`, `cross`, `normalize`, `sqrt`, `min/max/
//  clamp`, `wst::Pi`, ... all resolve identically once a file does:
//
//      #include ".../common/Common.h"
//      using namespace wst;
//
//  Nothing here uses MSL-only constructs, so any file built on this vocabulary is
//  compilable (and unit-testable) on the host as well as the GPU.
//

#pragma once

#ifdef __METAL_VERSION__
    #include <metal_stdlib>
#else
    #include <simd/simd.h>
    #include <cstdint>
    #include <cmath>
    #include <algorithm>
#endif

namespace wst {
#ifdef __METAL_VERSION__
    using namespace metal;                       // float3, dot, cross, normalize, sqrt, ...

    using int_t    = int;
    using uint_t   = uint;
    using float2   = metal::float2;
    using float3   = metal::float3;
    using float4   = metal::float4;
    using float4x4 = metal::float4x4;

    constant float Pi    = 3.14159265358979323846f;
    constant float InvPi = 0.31830988618379067154f;
#else
    using namespace simd;                        // float3, dot, cross, normalize, length, ...
    using std::sqrt;  using std::pow;
    using std::sin;   using std::cos;
    using std::abs;   // min / max / clamp come from simd (they cover scalar + vector);
                      // pulling std::{min,max,clamp} too made scalar calls ambiguous

    using int_t    = int32_t;
    using uint_t   = uint32_t;
    using float2   = simd_float2;
    using float3   = simd_float3;
    using float4   = simd_float4;
    using float4x4 = simd_float4x4;

    constexpr float Pi    = 3.14159265358979323846f;
    constexpr float InvPi = 0.31830988618379067154f;
#endif
}

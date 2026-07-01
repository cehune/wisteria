//
//  GeometryUtils.hpp
//  wisteria
//
//  Created by celine on 2026-06-30.
//

#pragma once
#include <Metal/Metal.hpp>
#include <simd/simd.h>

inline MTL::PackedFloat4x3 from4x4toPacked4x3(const simd_float4x4& m) {
    auto col = [](simd_float4 c) { return MTL::PackedFloat3(c.x, c.y, c.z); };
    return MTL::PackedFloat4x3(col(m.columns[0]), col(m.columns[1]),
                               col(m.columns[2]), col(m.columns[3]));
}
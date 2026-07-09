//
//  ConductorPresets.hpp
//  wisteria
//
//  Named lookup table of measured complex refractive indices (eta, k) for common metals.
//

#pragma once
#include <simd/simd.h>
#include <string>

namespace ConductorPresets {

struct IOR {
    simd_float3 eta;   // real part index of refraction
    simd_float3 k;     // extinction coefficient
};

// Case-insensitive substring match against table of known iors
bool lookup(const std::string& materialName, IOR& out);

} // namespace ConductorPresets

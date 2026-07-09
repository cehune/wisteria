//
//  ConductorPresets.cpp
//  wisteria
//
//  Created by celine on 2026-07-09.
//

#include "ConductorPresets.hpp"
#include <algorithm>
#include <cctype>

namespace {

struct Preset {
    const char* name;
    simd_float3 eta;
    simd_float3 k;
};

// RGB-sampled complex IOR, roughly following standard measured references
const Preset kPresets[] = {
    { "gold",     { 0.143f, 0.375f, 1.442f }, { 3.983f, 2.386f, 1.603f } },
    { "silver",   { 0.155f, 0.116f, 0.138f }, { 4.820f, 3.120f, 2.140f } },
    { "copper",   { 0.200f, 0.924f, 1.102f }, { 3.910f, 2.450f, 2.140f } },
    { "aluminum", { 1.345f, 0.965f, 0.617f }, { 7.470f, 6.400f, 5.300f } },
};

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

} // namespace

bool ConductorPresets::lookup(const std::string& materialName, IOR& out) {
    std::string lower = toLower(materialName);
    for (const auto& p : kPresets) {
        if (lower.find(p.name) != std::string::npos) {
            out.eta = p.eta;
            out.k   = p.k;
            return true;
        }
    }
    return false;
}

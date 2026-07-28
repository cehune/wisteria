//
//  Spectrum.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include "Common.h"
using namespace wst;

using Spectrum = float3; // TODO swap for spectral with intensity, wave

inline bool is_black(Spectrum c) { return all(c == float3(0.0f)); }

// luminance is perceived brightness weighted on known human sensitivity values
inline float luminance(float3 c) { return dot(c, float3{0.2126f, 0.7152f, 0.0722f}); } 

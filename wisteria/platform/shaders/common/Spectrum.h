//
//  Spectrum.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include <metal_stdlib>
using namespace metal;

using Spectrum = float3; // TODO swap for spectral with intensity, wave
inline float luminance(Spectrum c) { return dot(c, float3(0.2126, 0.7152, 0.0722)); }
inline bool  is_black (Spectrum c) { return all(c == 0.0f); }

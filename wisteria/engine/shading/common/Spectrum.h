//
//  Spectrum.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include "Math.h"
using namespace wst;

using Spectrum = float3; // TODO swap for spectral with intensity, wave

inline bool is_black(Spectrum c) { return all(c == float3(0.0f)); }

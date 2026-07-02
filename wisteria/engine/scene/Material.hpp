//
//  Material.hpp
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include <cstdint>
#include "engine/gpu/SharedTypes.h"   // struct Material (shared CPU/GPU ABI)

enum MaterialType : uint32_t { MATERIAL_LAMBERTIAN = 0 };

// The Material struct lives in engine/gpu/SharedTypes.h — one definition compiled by
// both C++ and Metal, so the layout can't drift. This header keeps the enum.

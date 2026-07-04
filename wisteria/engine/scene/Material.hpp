//
//  Material.hpp
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include "engine/shading/common/SharedTypes.h"   // struct Material + enum MaterialType (shared ABI)

// Material and MaterialType both live in engine/shading/common/SharedTypes.h — one
// definition compiled by both C++ and Metal, so the layout and enum values can't drift.

//
//  BsdfSample.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include "../common/Common.h"
using namespace wst;

struct BSDFSample {
    float3 wi;      // direction
    float3 f;       // color
    float  pdf;     // solid-angle pdf (a discrete lobe-selection probability when isDelta)
    bool   isDelta; // mirror, for smooth lobe
};

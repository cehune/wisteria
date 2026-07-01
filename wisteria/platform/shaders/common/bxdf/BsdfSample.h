//
//  BsdfSample.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include <metal_stdlib>

struct BSDFSample {
    float3 wi; // direction
    float3 f; // color
    float pdf; // cosine weighted pdf
};

//
//  Sampler.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include <metal_stdlib>
using namespace metal;

/*
Samplers have to follow GPU style polymorphism instead of inheritence,
which means no abstract interfaces because the GPU has to know exact sizes 
*/

struct IndependentSampler { uint state; };

// PCG (Wellons) advance, pseudo random number for now
//https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
inline uint pcg_next(thread IndependentSampler& s) {
    uint x = s.state;
    s.state = x * 747796405u + 2891336453u;
    uint word = ((x >> ((x >> 28u) + 4u)) ^ x) * 277803737u;
    return (word >> 22u) ^ word;
}
inline float  next_1d(thread IndependentSampler& s) {
    return float(pcg_next(s)) * (1.0f / 4294967296.0f);
}
inline float2 next_2d(thread IndependentSampler& s) {
    return float2(next_1d(s), next_1d(s));
}

// lowbias32 hash for seeding
inline uint hash_u(uint x) {
    x ^= x >> 16; x *= 0x7feb352du; x ^= x >> 15; x *= 0x846ca68bu; x ^= x >> 16; return x;
}
inline IndependentSampler make_sampler(uint2 p, uint sample) {
    IndependentSampler s;
    s.state = hash_u(p.x * 1973u ^ p.y * 9277u ^ sample * 26699u) | 1u; // nonzero
    return s;
}

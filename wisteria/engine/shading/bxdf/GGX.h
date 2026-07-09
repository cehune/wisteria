//
//  GGX.h
//  wisteria
//
//  Created by celine on 2026-07-08.
//

#pragma once
#include "../common/Common.h"
#include "../common/SharedTypes.h"
#include "BsdfSample.h"

using namespace wst;

/*
 NDF equation for microfacet normals.
*/
inline float ggx_ndf(float3 m, float alpha) {
    // actual equation has m dot n (microscopic dot macroscopic normals))
    // to get the cosine alignment
    // but if n is transformed back to the local normal its just (0,0,1)
    // alpha is the material roughness
    // this is the dot cosine rex
    float normalCos = m.z;
    float alpha2 = alpha * alpha;
    float roughnessFactor = alpha2 - 1;
    float denom = (normalCos * normalCos) * roughnessFactor + 1;
    
    return alpha2 * (1 / (Pi * denom * denom));
}

// cloaking ratio for a single direction w given the surface roughness alpha
inline float ggx_lambda(float3 w, float alpha) {
    float cosTheta = w.z;
    if (cosTheta <= 0.0f) return 0.0f;

    float cosTheta2 = cosTheta * cosTheta;
    float alpha2    = alpha * alpha;
    
    // pure geometric tangent calculation: tan^2 = sin^2 / cos^2
    float tanTheta2 = (1.0f - cosTheta2) / cosTheta2;
    
    // Smith Lambda func
    float numerator = -1.0f + sqrt(1.0f + alpha2 * tanTheta2);
    return numerator * 0.5f;
}

// Height-Correlated Smith G2 Masking-Shadowing Function.
inline float ggx_g2(float3 wo, float3 wi, float alpha) {
    // If either ray goes below the horizon, visibility is zero.
    if (wo.z <= 0.0f || wi.z <= 0.0f) return 0.0f;

    return 1.0f / (1.0f + ggx_lambda(wo, alpha) + ggx_lambda(wi, alpha));
}

// Single-direction visibility (G1) for a ray

inline float ggx_g1(float3 w, float alpha) {
    return 1.0f / (1.0f + ggx_lambda(w, alpha));
}

// VNDF spherical cap sampler
// High-Performance Graphics (2023) Isotropic Visible GGX Normal Sampler.
// Assumes wo.z >= 0 (callers with two-sided directions must warp into the
// upper hemisphere first and flip the result back, as Conductor/Dielectric do).
inline float3 ggx_sample_vndf(float3 wo, float alpha, float2 u) {
    // warp outgoing vector into the elongated hemisphere config
    float3 woStd = normalize(float3{wo.x * alpha, wo.y * alpha, wo.z});

    // uniformly sample a spherical cap slice matching the elevation of woStd.z
    float phi = (2.0f * u.x - 1.0f) * Pi;
    float z   = (1.0f - u.y) * (1.0f + woStd.z) - woStd.z;

    float sinTheta = sqrt(wst::max(0.0f, 1.0f - z * z));
    float x = sinTheta * cos(phi);
    float y = sinTheta * sin(phi);
    float3 c = float3{x, y, z};

    // standard-space microfacet normal is the unnormalized half-vector
    float3 mStd = c + woStd;

    // warp back to the physical un-elongated space and normalize
    return normalize(float3{mStd.x * alpha, mStd.y * alpha, mStd.z});
}
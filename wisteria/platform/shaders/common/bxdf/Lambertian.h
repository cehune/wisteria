//
//  Lambertian.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include <metal_stdlib>
#include "../Spectrum.h"
#include "../Frame.h"
#include "BsdfSample.h"
#include "../Warp.h"

// reflectance factor even across a sphere
inline Spectrum lambertian_eval(Spectrum albedo, float3 wo, float3 wi) {
    if (!sameHemisphere(wo, wi)) return Spectrum(0.0);
    return albedo * M_1_PI_F; // albedo / pi = fr
}

// pdf of the cosine-weighted sample
// remember that this is importance sampling to cancel out the
// cosine in the original energy integral
// cancels the cosine and pi terms in main integrator loop
inline float lambertian_pdf(float3 wo, float3 wi) {
    if (!sameHemisphere(wo, wi)) return 0.0f;
    return abs(cosTheta(wi)) * M_1_PI_F;
}

inline BSDFSample lambertian_sample(Spectrum albedo, float3 wo, float2 u) {
    float3 wi = square_to_cosine_hemisphere(u);
    if (wo.z < 0.0f) wi.z = -wi.z;
    
    BSDFSample bs;
    bs.wi = wi;
    bs.f = albedo * M_1_PI_F;
    bs.pdf = square_to_cosine_hemisphere_pdf(abs(cosTheta(wi)));
    return bs;
}

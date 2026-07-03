//
//  Lambertian.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include "../common/Math.h"
#include "../common/Spectrum.h"
#include "../common/Frame.h"
#include "../common/Warp.h"
#include "BsdfSample.h"
using namespace wst;

// reflectance factor even across a sphere
inline Spectrum lambertian_eval(Spectrum albedo, float3 wo, float3 wi) {
    if (!sameHemisphere(wo, wi)) return Spectrum(0.0);
    return albedo * InvPi; // albedo / pi = fr
}

// pdf of the cosine-weighted sample
// remember that this is importance sampling to cancel out the
// cosine in the original energy integral
// cancels the cosine and pi terms in main integrator loop
inline float lambertian_pdf(float3 wo, float3 wi) {
    if (!sameHemisphere(wo, wi)) return 0.0f;
    return abs(cosTheta(wi)) * InvPi;
}

inline BSDFSample lambertian_sample(Spectrum albedo, float3 wo, float2 u) {
    float3 wi = square_to_cosine_hemisphere(u);
    if (wo.z < 0.0f) wi.z = -wi.z;

    BSDFSample bs;
    bs.wi = wi;
    bs.f = albedo * InvPi;
    bs.pdf = square_to_cosine_hemisphere_pdf(abs(cosTheta(wi)));
    return bs;
}

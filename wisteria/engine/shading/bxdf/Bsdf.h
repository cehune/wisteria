//
//  Bsdf.h
//  wisteria
//
//  BxDF dispatch: the integrator talks to bsdf_sample / bsdf_eval / bsdf_pdf and never
//  names a concrete BRDF. Routing is on Material.type. All three share the same
//  BSDFSample / solid-angle-pdf contract, so MIS works uniformly across materials.
//

#pragma once
#include "../common/Common.h"
#include "../common/Spectrum.h"
#include "../common/SharedTypes.h"   // Material, MaterialType
#include "BsdfSample.h"
#include "Lambertian.h"
#include "Conductor.h"
#include "Dielectric.h"
using namespace wst;

/*
Mind that the args for each of the material types are going to be different!
*/

// perceptual roughness -> GGX alpha, clamped away from 0 (true mirror = a delta, TODO).
inline float bsdf_alpha(Material mat) {
    float r = max(mat.roughness, 1e-3f);
    return r * r;
}

// uDiscrete: extra 1D sample used only by materials that stochastically choose between
// multiple lobes (e.g. dielectric reflection vs. transmission). Ignored otherwise.
inline BSDFSample bsdf_sample(Material mat, float3 wo, float2 u, float uDiscrete) {
    if (mat.type == MATERIAL_CONDUCTOR)
        return conductor_sample(mat.albedo, bsdf_alpha(mat), wo, u);
    if (mat.type == MATERIAL_DIELECTRIC)
        return dielectric_sample(mat.albedo, bsdf_alpha(mat), mat.eta, wo, u, uDiscrete);
    return lambertian_sample(mat.albedo, wo, u);
}

inline Spectrum bsdf_eval(Material mat, float3 wo, float3 wi) {
    if (mat.type == MATERIAL_CONDUCTOR)
        return conductor_eval(mat.albedo, bsdf_alpha(mat), wo, wi);
    if (mat.type == MATERIAL_DIELECTRIC)
        return dielectric_eval(mat.albedo, bsdf_alpha(mat), mat.eta, wo, wi);
    return lambertian_eval(mat.albedo, wo, wi);
}

inline float bsdf_pdf(Material mat, float3 wo, float3 wi) {
    if (mat.type == MATERIAL_CONDUCTOR)
        return conductor_pdf(bsdf_alpha(mat), wo, wi);
    if (mat.type == MATERIAL_DIELECTRIC)
        return dielectric_pdf(bsdf_alpha(mat), mat.eta, wo, wi);
    return lambertian_pdf(wo, wi);
}

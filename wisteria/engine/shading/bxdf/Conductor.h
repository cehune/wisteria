//
//  Conductor.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//
//  Isotropic GGX (Trowbridge-Reitz) microfacet conductor, built up piece by piece:
//    NDF (D)  ->  masking (G)  ->  Fresnel (F)  ->  eval  ->  sample + pdf
//  Local shading frame: +z is the surface normal. Portable ops only (host-testable).
//

#pragma once
#include "../common/Common.h"
#include "../common/SharedTypes.h"
#include "GGX.h"
#include "BsdfSample.h"

using namespace wst;

/*
 Isotropic GGX (Trowbridge-Reitz) Microfacet Conductor BSDF

 THE BIG ASSUMPTION IS THAT NORMAL IS LOCAL COORDS (0,0,1)!!!
 THATS WHY YOU SEE SO MANY W{}.Z OCCURANCES
 
 Heres the loop
 this is for metallic surfaces
 Its not random outgoing light like it is for lambertian, there are cases 
 of specular transport here.

 Direct lighting is pretty normal
 conductor eval is the brdf throughput
 conductor pdf is the solid angle scattering density
 obviously we combine with MIS with the ligths PDF 

 direction is gotten using conductor sample
 this generates a microfacet (m) (half vector between ingoing and outgoing) vector
 and a macrofacet vector (just the normal)
 and then reflects the incoming vector wo across m to get the outgoing wi

 All the core equations are part of the torrance-sparrow specular brdf equation
 
 1. Normal Distribution Function (NDF) - Isotropic GGX:
 Measures the sub-grid surface area density pointing along 'm'.
 
 2. Joint Masking-Shadowing Term - Height-Correlated Smith G2:
 Measures the fraction of microfacets mutually visible from both 'wo' and 'wi'.

 3. Wavelength Reflection Profile - Schlick Conductor Fresnel:
 Approximates metallic color shift at grazing angles using albedo as F0.

 4. Probability Density Function - Visible Normal (VNDF) Solid Angle PDF:
 */

// Schlick's approximation for conductors, using the albedo as the base
// is a bit of an approx for material properties
inline float3 fresnel_conductor(float3 albedo, float cosThetaH) {
    float c = 1.0f - cosThetaH;
    float c5 = c * c * c * c * c; // c^5
    return albedo + (float3(1.0f) - albedo) * c5;
}

inline float3 conductor_eval(float3 albedo, float alpha, float3 wo, float3 wi) {
    if (wo.z <= 0.0f || wi.z <= 0.0f) return float3(0.0f);
    
    // calculate micronormal half vector
    float3 m = normalize(wo + wi);
    
    // eval
    float D = ggx_ndf(m, alpha);
    float G = ggx_g2(wo, wi, alpha);
    float3 F = fresnel_conductor(albedo, dot(wo, m));
    
    // assemble torrance sparrow microfacet
    float3 numerator = D * G * F;
    // wo.z and wi.z are the dots of wo and wi to the normal
    // but assume that the normal is 0,0,1 in local
    float denominator = 4.0f * wo.z * wi.z;
    return numerator * (1.0f / denominator);
}

inline float conductor_pdf(float alpha, float3 wo, float3 wi) {
    if (wo.z <= 0.0f || wi.z <= 0.0f) return 0.0f;
    
    // calculate the micronormal half vector
    float3 m = normalize(wo + wi);
    
    // evaluate the visible normal pdf
    float D = ggx_ndf(m, alpha);
    float G1 = ggx_g1(wo, alpha);
    
    // divide by the reflectance jacobian
    // wo.z is the dot of wo to the normal
    // but assume that the normal is 0,0,1 in local
    return (D * G1) * (1.0f / (4.0f * wo.z));
}

// Contract for conductors
// Samples a reflection ray, returning the vector, evaluated BRDF, and solid-angle PDF.
inline BSDFSample conductor_sample(float3 albedo, float alpha, float3 wo, float2 u) {
    BSDFSample bs;
    bs.wi      = float3(0.0f);
    bs.f       = float3(0.0f);
    bs.pdf     = 0.0f;
    bs.isDelta = false;   // GGX conductor is always a continuous (glossy) lobe

    if (wo.z <= 0.0f) return bs;
    
    // generate the microfacet normal
    // needs to be physically informed which is why we cant just sample a random direction
    float3 m = ggx_sample_vndf(wo, alpha, u);
    
    // reflect wo across the microfacet normal to get the incoming ray wi
    bs.wi = normalize(2.0f * dot(wo, m) * m - wo);
    
    // reject reflections that bounce beneath the geometric surface
    if (bs.wi.z <= 0.0f) return bs;
    
    // pdf and evaluation
    bs.pdf = conductor_pdf(alpha, wo, bs.wi);
    bs.f   = conductor_eval(albedo, alpha, wo, bs.wi);
    
    return bs;
}

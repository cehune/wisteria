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

// VNDF spherical cap sampler
// High-Performance Graphics (2023) Isotropic Visible GGX Normal Sampler.
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

// Contract for conductors
// Samples a reflection ray, returning the vector, evaluated BRDF, and solid-angle PDF.
inline BSDFSample conductor_sample(float3 albedo, float alpha, float3 wo, float2 u) {
    BSDFSample bs;
    bs.wi  = float3(0.0f);
    bs.f   = float3(0.0f);
    bs.pdf = 0.0f;
    
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

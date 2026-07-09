//
//  Dielectric.h
//  wisteria
//
//  Created by celine on 2026-07-08.
//

#pragma once
#include "../common/Common.h"
#include "../common/SharedTypes.h"
#include "GGX.h"
#include "BsdfSample.h"

/*
dialectric is built on the fact that we partially split (bounce and enter)
dialectrics have an eta which is the index of reflaction. Basically how much
it slows down light. 
*/

struct DielectricDirectionComponents {
  float etaI;
  float etaT;
  float3 m;
  bool isReflection;
  bool isValid; // <-- Centralized sanity check
};

inline DielectricDirectionComponents getDielectricDirectionComponents(float eta, float3 wo, float3 wi) {
  DielectricDirectionComponents d;
  d.isValid = true;

  // Guard against edge-case division by zero
  if (wo.z == 0.0f || wi.z == 0.0f) {
      d.isValid = false;
      return d;
  }

  d.etaI = (wo.z > 0.0f) ? 1.0f : eta;
  d.etaT = (wo.z > 0.0f) ? eta : 1.0f;
  // check if reflection or transmission
  d.isReflection = (wo.z * wi.z > 0.0f); // if on same axis, its reflection

  // microfacet normal m
  if (d.isReflection) {
    d.m = normalize(wo + wi);
  } else {
    d.m = normalize(-(wo * d.etaI + wi * d.etaT));
  }
  if (dot(wo, d.m) < 0.0f) {
      d.m = -d.m;
  }

  // these are really important checks that were screwing me up
  // 1. Macro-horizon check: m tracks wo's hemisphere
  // so a valid m must have the SAME sign as wo.z on both sides, not just when entering.
  if ((d.m.z * wo.z) <= 0.0f) {
      d.isValid = false;
  }

  // 2. Transmission / Reflection sanity checks
  float cosWi = dot(wi, d.m);
  if (d.isReflection) {
      // For reflection, both rays must be on the same side of the microfacet (this is
      // always true by construction for the reflection bisector -- kept for symmetry).
      if (cosWi <= 0.0f) d.isValid = false;
  } else {
      // For transmission, rays must be on opposite sides of the microfacet.
      if (cosWi >= 0.0f) d.isValid = false;
  }

  return d;
}

struct FresnelResult {
  float F;
  float cosThetaT;
};

/*
btdf, reuses the ggx terms, calculate the energy
cosThetaI: angle incoming relative to the surface normal
etaI: incident (where light is coming from)
etaT: outgoing / transmission (where light is going into (the mat))

output cosThetaT: angle outgoing relative to the surface normal
*/
inline FresnelResult fresnel_dielectric(float cosThetaI, float etaI, float etaT) {
  cosThetaI = clamp(cosThetaI, -1.0f, 1.0f);

  // Swap etas if we are hitting the back side of the microfacet
  if (cosThetaI < 0.0f) {
      float tmp = etaI; etaI = etaT; etaT = tmp;
      cosThetaI = -cosThetaI;
  }

  // snells law, interior angle, determines how much it should refract inside
  float sinThetaI = max(0.0f, 1.0f - cosThetaI * cosThetaI);
  float sinThetaT = (etaI / etaT) * (etaI / etaT) * sinThetaI;

  // total internal reflection check, big pass around!!
  if (sinThetaT >= 1.0f) {
        return { 1.0f, 0.0f };
    }

  float cosThetaT = sqrt(1.0f - sinThetaT);

  // fresnel terms, describe reflection and transmission of light at boundary
  // between media. Parallel is the component of light vibrating parallel to the plane of incidence,
  // perp is obviously perpindicular. Result is the total reflectance, as average of squares.
  float rParallel = ((etaT * cosThetaI) - (etaI * cosThetaT)) / (((etaT * cosThetaI) + (etaI * cosThetaT)));
  float rPerpindicular = ((etaI * cosThetaI) - (etaT * cosThetaT)) / (((etaI * cosThetaI) + (etaT * cosThetaT)));
  float F = (rParallel * rParallel + rPerpindicular * rPerpindicular) * 0.5f;
  return { F, cosThetaT };
}

/*
at it's core, its geometric correction * microfacet statistic * fresnel energy * snells correction
F is fresnel_dielectric
D is ggx_ndf
G is ggx_g2 (DG = microfacet statistic)

we have to call this for both reflection or transmission through the material, so what we can do 
is determine it based on wo.z
*/
inline float3 dielectric_eval(float3 albedo, float alpha, float eta, float3 wo, float3 wi) {
  // 1. geometric correction
  // Determine which medium we are coming from based on wo.z
  DielectricDirectionComponents d = getDielectricDirectionComponents(eta, wo, wi);
  if (!d.isValid) return float3(0.0f);

  // microfacet statistic
  float D = ggx_ndf(d.m, alpha);
  float G = ggx_g2(abs(wo), abs(wi), alpha);

  // fresnel energy
  FresnelResult fr = fresnel_dielectric(dot(wo, d.m), d.etaI, d.etaT);
  float F = fr.F;

  if (d.isReflection) {
        // Torrance-Sparrow reflection formula
        float denom = 4.0f * abs(wo.z) * abs(wi.z);
        return float3(F * D * G / denom) * albedo;
    } else {
        // Walter transmission formula
        float sqrtDenom = d.etaI * dot(wo, d.m) + d.etaT * dot(wi, d.m);
        // based on the snells correction
        float denom = (sqrtDenom * sqrtDenom) * abs(wo.z) * abs(wi.z);

        float factor = (abs(dot(wo, d.m)) * abs(dot(wi, d.m))) / denom;
        // radiance-scaled path throughput, not the raw bsdf lol
        // (radiance transport compresses by (etaIncident/etaTransmitted)^2, not the other
        // way around -- see PBRT DielectricBxDF::f's `ft /= Sqr(etap)`)
        float3 throughput = float3((1.0f - F) * D * G * factor * (d.etaI * d.etaI));
        
        return throughput * albedo; // Tint transmission with material color
    }
}

// 
inline float dielectric_pdf(float alpha, float eta, float3 wo, float3 wi) {
    DielectricDirectionComponents d = getDielectricDirectionComponents(eta, wo, wi);
    if (!d.isValid) return 0.0f;

    float D = ggx_ndf(d.m, alpha);
    float G1 = ggx_g1(abs(wo), alpha);
    FresnelResult fr = fresnel_dielectric(abs(dot(wo, d.m)), d.etaI, d.etaT);

    // Probability of choosing reflection vs transmission
    float probReflection = fr.F;

    if (d.isReflection) {
        return (D * G1 / (4.0f * abs(wo.z))) * probReflection;
    } else {
        float sqrtDenom = d.etaI * dot(wo, d.m) + d.etaT * dot(wi, d.m);
        if (abs(sqrtDenom) < 1e-5f) return 0.0f;

        float jacobian = (d.etaT * d.etaT * abs(dot(wi, d.m))) / (sqrtDenom * sqrtDenom);
        float pdf_m = (D * G1 * abs(dot(wo, d.m))) / abs(wo.z);
        
        return pdf_m * jacobian * (1.0f - probReflection);
    }
}

inline BSDFSample dielectric_sample(float3 albedo, float alpha, float eta, float3 wo, float2 u, float uDiscrete) {
    BSDFSample bs;
    bs.wi      = float3(0.0f);
    bs.f       = float3(0.0f);
    bs.pdf     = 0.0f;
    bs.isDelta = false;

    if (wo.z == 0.0f) return bs;

    // warp wo to upper hemisphere for VNDF sampling if it is coming from below
    float3 woVNDF = wo;
    if (woVNDF.z < 0.0f) woVNDF = -woVNDF;

    // generate microfacet normal m using existing VNDF sampler
    float3 m = ggx_sample_vndf(woVNDF, alpha, u);
    if (wo.z < 0.0f) m = -m; // Flip back to match the tracking hemisphere

    // determine relative IOR context
    float etaI = (wo.z > 0.0f) ? 1.0f : eta;
    float etaT = (wo.z > 0.0f) ? eta : 1.0f;

    FresnelResult fr = fresnel_dielectric(abs(dot(wo, m)), etaI, etaT);

    // stochastically choose Reflection or Refraction
    if (uDiscrete < fr.F) {
        // Reflection
        bs.wi = normalize(2.0f * dot(wo, m) * m - wo);
        if (wo.z * bs.wi.z <= 0.0f) return bs; // Guard against macro-horizon failure
    } else {
        // Refraction (Snell's Law)
        float etaRatio = etaI / etaT;
        float cosThetaI = dot(wo, m);
        bs.wi = normalize((etaRatio * cosThetaI - fr.cosThetaT) * m - etaRatio * wo);
        if (wo.z * bs.wi.z >= 0.0f) return bs; // Must cross macro-horizon
    }

    // 5. Evaluate final weightings
    bs.pdf = dielectric_pdf(alpha, eta, wo, bs.wi);
    bs.f   = dielectric_eval(albedo, alpha, eta, wo, bs.wi);

    return bs;
}

inline BSDFSample dielectric_smooth_sample(float3 albedo, float eta, float3 wo, float uDiscrete) {
    BSDFSample bs;
    bs.wi      = float3(0.0f);
    bs.f       = float3(0.0f);
    bs.pdf     = 0.0f;
    bs.isDelta = true; // specular

    if (wo.z == 0.0f) return bs;

    // microfacet normal is exactly the surface normal
    float3 m = float3{0.0f, 0.0f, sign(wo.z)};    
    if (wo.z < 0.0f) m = -m; // Flip back to match the tracking hemisphere

    // determine relative IOR context
    float etaI = (wo.z > 0.0f) ? 1.0f : eta;
    float etaT = (wo.z > 0.0f) ? eta : 1.0f;

    // fresnel reflectance
    FresnelResult fr = fresnel_dielectric(abs(dot(wo, m)), etaI, etaT);

    // stochastically choose Reflection or Refraction
    if (uDiscrete < fr.F) {
        // Reflection
        bs.wi = float3{-wo.x, -wo.y, wo.z};
        // wo.z is the cosine
        bs.f = albedo * fr.F * (1.0f / abs(wo.z));
        bs.pdf = fr.F;
    } else {
        // Refraction (Snell's Law)
        float etaRatio = etaI * (1.0f / etaT);
        bs.wi.x = -etaRatio * wo.x;
        bs.wi.y = -etaRatio * wo.y;
        bs.wi.z  = -fr.cosThetaT * sign(wo.z);

        bs.f = albedo * (1.0f - fr.F) * (etaRatio * etaRatio) * (1.0f / abs(bs.wi.z));
        bs.pdf   = 1 - fr.F;
    }
    return bs;
}

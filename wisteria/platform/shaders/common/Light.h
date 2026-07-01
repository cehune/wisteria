//
//  Light.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

enum LightType { AREA, ENVIRONMENT, POINT, DIRECTIONAL };
struct Light {
    uint     type;
    Spectrum radiance;   // Le (area/env) or intensity (point/dir)
    uint     instanceID; // AREA: emissive geometry (+ area CDF if multi-triangle)
    bool     twoSided;   // AREA
    float3   posOrDir;   // POINT: position DIRECTIONAL: direction   ENV: unused
    uint     flags;      // isDelta, isInfinite
};

LightSample light_sample_Li(Light L, float3 refP, float2 u); // NEE: { wi, Li, pdfω, dist }
float       light_pdf_Li  (Light L, float3 refP, float3 wi); // MIS: pdf a BSDF-sampled wi came from light sampling
Spectrum    light_Le      (Light L, Ray  r);                 // BSDF-side MIS: env miss / bounce hits an area light

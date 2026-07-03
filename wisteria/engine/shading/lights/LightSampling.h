//
//  LightSampling.h
//  wisteria
//
//  Pure light samplers: given world-space light geometry + a reference point, return
//  a LightSample (direction, incident radiance, solid-angle pdf, distance). No device
//  buffers — the platform seam fetches the area-light triangle and calls in, which
//  keeps these host-unit-testable (sample<->pdf reciprocity). The 1/numLights uniform
//  light-selection factor is applied by the caller (light_sample_Li).
//

#pragma once
#include "../common/Math.h"
#include "../common/Spectrum.h"
#include "LightGeometry.h"      // area_pdf_direction
using namespace wst;

struct LightSample {
    float3   wi;    // refP -> light, normalized
    Spectrum Li;    // radiance arriving at refP
    float    pdf;   // solid-angle pdf (per-light; excludes 1/numLights)
    float    dist;  // distance to the sampled point (shadow-ray tmax)
};

// Uniform-area sample of one triangle (A,B,C) of an area light, seen from refP.
// numTri = triangle count of the light mesh (uniform triangle-selection pdf factor).
inline LightSample light_sample_area(float3 A, float3 B, float3 C,
                                     Spectrum radiance, bool twoSided,
                                     float3 refP, float2 bary, uint numTri)
{
    // uniform-area barycentric warp (pbrt UniformSampleTriangle).
    float  su0 = sqrt(bary.x);
    float  b0  = 1.0f - su0;
    float  b1  = bary.y * su0;
    float3 lp  = b0 * A + b1 * B + (1.0f - b0 - b1) * C;

    float3 n       = normalize(cross(B - A, C - A));
    float3 toLight = lp - refP;
    float  dist    = length(toLight);
    float3 wi      = toLight / dist;

    LightSample s;
    s.wi   = wi;
    s.dist = dist;
    s.pdf  = area_pdf_direction(A, B, C, refP, lp, float(numTri));
    s.Li   = (twoSided || dot(-wi, n) > 0.0f) ? radiance : Spectrum(0.0f);
    return s;
}

// Delta lights: pdf = 1 (the caller still divides by numLights to stay consistent).
inline LightSample light_sample_point(float3 pos, Spectrum radiance, float3 refP)
{
    float3 toLight = pos - refP;
    float  dist    = length(toLight);
    LightSample s;
    s.wi   = toLight / dist;
    s.Li   = radiance;
    s.pdf  = 1.0f;
    s.dist = dist;
    return s;
}

inline LightSample light_sample_directional(float3 dir, Spectrum radiance)
{
    LightSample s;
    s.wi   = -dir;
    s.Li   = radiance;
    s.pdf  = 1.0f;
    s.dist = INFINITY;
    return s;
}

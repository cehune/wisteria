//
//  LightGeometry.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//
//  Pure light-geometry math shared by the Metal shaders and host C++ unit tests:
//  the single source of truth for the area-light -> solid-angle density. The NEE
//  sampler (light_sample_area) and the integrator's emitter-hit MIS weight both call
//  area_pdf_direction, so the sampling density and the MIS pdf cannot drift.
//

#pragma once
#include "../common/Common.h"
using namespace wst;

// Solid-angle pdf of choosing point `lp` on triangle (a,b,c) by uniform-area
// sampling, as seen from `refP`, for an area light of `numTri` triangles picked
// uniformly. Measure: solid angle at refP — mirrors Mitsuba's Shape::pdf_direction.
// EXCLUDES the 1/numLights selection factor; the caller applies it (light_sample_Li).
// Returns 0 for a degenerate (zero-area) triangle or a coincident reference point.
inline float area_pdf_direction(float3 a, float3 b, float3 c,
                                float3 refP, float3 lp, float numTri)
{
    // n = (b-a) x (c-a), ||n|| = 2 * triangle area
    float3 e1 = b - a;
    float3 e2 = c - a;
    float3 n;
    n.x = e1.y * e2.z - e1.z * e2.y;
    n.y = e1.z * e2.x - e1.x * e2.z;
    n.z = e1.x * e2.y - e1.y * e2.x;

    float twoArea = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
    if (twoArea <= 0.0f) return 0.0f;           // degenerate triangle
    float area = 0.5f * twoArea;

    float3 d;
    d.x = lp.x - refP.x;
    d.y = lp.y - refP.y;
    d.z = lp.z - refP.z;
    float dist2 = d.x * d.x + d.y * d.y + d.z * d.z;
    if (dist2 <= 0.0f) return 0.0f;             // reference point on the light
    float dist = sqrt(dist2);

    // cosL = |wi . unitNormal| = |(d/dist) . (n/twoArea)|
    float cosL = (d.x * n.x + d.y * n.y + d.z * n.z) / (dist * twoArea);
    cosL = cosL < 0.0f ? -cosL : cosL;
    if (cosL < 1e-4f) cosL = 1e-4f;             // clamp edge-on; matches sampler

    // area-measure pdf 1/(numTri*area), times the dA->dw Jacobian dist2/cosL
    return dist2 / (numTri * area * cosL);
}

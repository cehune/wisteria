//
//  LightGeometry.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//
//  Pure light-geometry math shared by the Metal shaders and the host C++ unit
//  tests: the single source of truth for the area-light -> solid-angle density.
//  The NEE sampler (light_sample_area) and the MIS pdf (light_pdf_direction) both
//  derive from area_pdf_direction, so the two cannot drift.
//
//  Lives in engine/gpu (not platform/shaders) for the reason SharedTypes.h states:
//  it has NO MSL-only constructs (no `device`, no [[attributes]]). It uses the
//  project's WST_* vector alias so it compiles unchanged under Metal and C++/simd.
//

#pragma once
#include "SharedTypes.h"
#ifndef __METAL_VERSION__
    #include <cmath>
#endif

/*
solid angle pdf of choosing a point lightPoint on a triangle (A, B, C)
equiv to Mitsuba's Shape::pdf_direction
TODO: switch to hemisphere non uniform sampling for area lights
*/
inline float area_pdf_direction(WST_FLOAT3 A, WST_FLOAT3 B, WST_FLOAT3 C,
                                WST_FLOAT3 surfacePoint, WST_FLOAT3 lightPoint, float numTri)
{
    // n = (b-a) x (c-a), ||n|| = 2 * triangle area
    WST_FLOAT3 e1 = B - A;
    WST_FLOAT3 e2 = C - A;
    WST_FLOAT3 n;
    n.x = e1.y * e2.z - e1.z * e2.y;
    n.y = e1.z * e2.x - e1.x * e2.z;
    n.z = e1.x * e2.y - e1.y * e2.x;

    float twoArea = sqrt(n.x * n.x + n.y * n.y + n.z * n.z);
    if (twoArea <= 0.0f) return 0.0f;           // degenerate triangle
    float area = 0.5f * twoArea;

    WST_FLOAT3 D;
    D.x = lightPoint.x - surfacePoint.x;
    D.y = lightPoint.y - surfacePoint.y;
    D.z = lightPoint.z - surfacePoint.z;
    float dist2 = D.x * D.x + D.y * D.y + D.z * D.z;
    if (dist2 <= 0.0f) return 0.0f;
    float dist = sqrt(dist2);

    // cosL = |wi . unitNormal| = |(d/dist) . (n/twoArea)|
    float cosL = (D.x * n.x + D.y * n.y + D.z * n.z) / (dist * twoArea);
    cosL = cosL < 0.0f ? -cosL : cosL;
    if (cosL < 1e-4f) cosL = 1e-4f; // clamp

    // area-measure pdf 1/(numTri*area), times the dA->dw Jacobian dist2/cosL
    return dist2 / (numTri * area * cosL);
}

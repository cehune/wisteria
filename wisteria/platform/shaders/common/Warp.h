//
//  Warp.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//
// Shared sampling warps: map uniform [0,1)^n samples onto useful domains.

#pragma once
#include <metal_stdlib>
using namespace metal;


/*
We start with a uniform square with both parts being able to be
0-1, then map it to a uniform 2D distribution across a unit disk
*/
inline float2 square_to_uniform_disk(float2 u) {
    // remap [0,1] to [-1,1] R2
    float2 o = 2.0f * u - 1.0f;
    if (o.x == 0.0f && o.y == 0.0f) return float2(0.0f);

    float r, theta;
    /*
     pi / 4 * y/x canculates an angle between -45 and 45
     then the sign (or 45 and 135, about the pos y axis)
     multiplying by r is just to use the sign to flip it in
     case the chosen value is negative. Unit angle will still be
     the same. also the value of r constrains the actual radius arm to 1.
     */
    if (abs(o.x) > abs(o.y)) {
        r     = o.x;
        theta = (M_PI_F / 4.0f) * (o.y / o.x);
    } else {
        r     = o.y;
        theta = (M_PI_F / 2.0f) - (M_PI_F / 4.0f) * (o.x / o.y);
    }
    return r * float2(cos(theta), sin(theta));
}


/*
Cosine weighted direction on the hemisphere around +z (local space)
z takes the point on the floor, and shoots it up vertically until
it intersects the surface of the 3D unit sphere (also mapped 0 to 1)
So this is constrained so that we get a point on a unit sphere lol
*/
inline float3 square_to_cosine_hemisphere(float2 u) {
    float2 d = square_to_uniform_disk(u);
    float  z = sqrt(max(0.0f, 1.0f - dot(d, d)));
    return float3(d.x, d.y, z);
}

// pdf of the direction above, in solid angle: cos(theta) / pi.
inline float square_to_cosine_hemisphere_pdf(float cosTheta) {
    return cosTheta * M_1_PI_F;
}

//
//  Frame.h
//  wisteria
//
//  Created by celine on 2026-07-01.
//

#pragma once
#include "Math.h"
using namespace wst;

struct Frame {
    float3 s; // tangent (x)
    float3 t; // bitangent (y)
    float3 n; // shading normal (z)

    static Frame fromNormalTangent(float3 normal, float3 tangent) {
        Frame f;
        f.n = normalize(normal);
        f.s = normalize(tangent - f.n * dot(f.n, tangent)); // project tangent into plane ⊥ n
        f.t = cross(f.n, f.s);                              // right-handed: s × t = n
        return f;
    }
    static Frame fromNormal(float3 normal) {
        Frame f; f.n = normalize(normal);
        float sign = (f.n.z >= 0.0f) ? 1.0f : -1.0f;
        float a = -1.0f / (sign + f.n.z);
        float b = f.n.x * f.n.y * a;
        f.s = float3(1.0f + sign * f.n.x * f.n.x * a, sign * b, -sign * f.n.x);
        f.t = float3(b, sign + f.n.y * f.n.y * a, -f.n.y);
        return f;
    }

    // Transforms a vector from world space to local space
    float3 toLocal(float3 v) const {
        return float3(dot(v, s), dot(v, t), dot(v, n));
    }

    // Transforms a vector from local space to world space
    float3 toWorld(float3 v) const {
        return v.x * s + v.y * t + v.z * n;
    }
};

// helpers in shading space
// assumes vector 'w' is already in local space

inline float cosTheta(float3 w) {
    return w.z;
}

inline float cos2Theta(float3 w) {
    return w.z * w.z;
}

inline float sin2Theta(float3 w) {
    // Using the identity: sin^2 + cos^2 = 1
    return max(0.0f, 1.0f - cos2Theta(w));
}

inline float sinTheta(float3 w) {
    return sqrt(sin2Theta(w));
}

inline float tanTheta(float3 w) {
    return sinTheta(w) / cosTheta(w);
}

inline float tan2Theta(float3 w) {
    return sin2Theta(w) / cos2Theta(w);
}

inline float cosPhi(float3 w) {
    float sinT = sinTheta(w);
    return (sinT == 0.0f) ? 1.0f : clamp(w.x / sinT, -1.0f, 1.0f);
}

inline float sinPhi(float3 w) {
    float sinT = sinTheta(w);
    return (sinT == 0.0f) ? 0.0f : clamp(w.y / sinT, -1.0f, 1.0f);
}

inline bool sameHemisphere(float3 w, float3 wp) {
    return w.z * wp.z > 0.0f;
}

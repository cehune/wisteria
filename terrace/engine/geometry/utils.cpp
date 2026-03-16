//
//  utils.cpp
//  terrace
//
//  Created by celine on 2026-03-15.
//

#include "vec3.hpp"
#include <iostream>
#include <random>

// --- Math Operations ---

float dot(const vec3& v, const vec3& u) {
    // Note: simd_dot handles 4 components.
    // If your w is always 0, this is perfect.
    return simd_dot(v.data, u.data);
}

vec3 cross_product(const vec3& v, const vec3& u) {
    // simd_cross only works on float3, so we convert and convert back
    simd_float3 a = {v.data.x, v.data.y, v.data.z};
    simd_float3 b = {u.data.x, u.data.y, u.data.z};
    simd_float3 res = simd_cross(a, b);
    return vec3((simd_float4){res.x, res.y, res.z, 0.0f});
}

vec3 hadamard_product(const vec3& v, const vec3& u) {
    // Component-wise multiplication (v.x * u.x, etc.)
    return vec3(v.data * u.data);
}

// --- Randomization ---

// Helper for generating random floats
static inline float random_float() {
    static std::uniform_real_distribution<float> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

static inline float random_float(float min, float max) {
    return min + (max - min) * random_float();
}

vec3 random_vector() {
    return vec3((simd_float4){random_float(), random_float(), random_float(), 0.0f});
}

vec3 random_vector_bounded(double min, double max) {
    return vec3((simd_float4){
        (float)random_float(min, max),
        (float)random_float(min, max),
        (float)random_float(min, max),
        0.0f
    });
}

vec3 random_vector_unit() {
    while (true) {
        auto p = random_vector_bounded(-1, 1);
        auto lensq = dot(p, p);
        if (1e-6 < lensq && lensq <= 1.0) {
            return p.normal_of();
        }
    }
}

vec3 random_vec_on_hemisphere(const vec3& normal) {
    vec3 on_unit_sphere = random_vector_unit();
    if (dot(on_unit_sphere, normal) > 0.0) {
        return on_unit_sphere;
    } else {
        return -on_unit_sphere;
    }
}

vec3 random_unit_aperture_loc() {
    while (true) {
        auto p = vec3((simd_float4){random_float(-1, 1), random_float(-1, 1), 0, 0});
        if (dot(p, p) < 1) return p;
    }
}

// --- Comparison and Debug ---

vec3 vec_max(const vec3& v, const vec3& u) {
    return vec3(simd_max(v.data, u.data));
}

vec3 vec_min(const vec3& v, const vec3& u) {
    return vec3(simd_min(v.data, u.data));
}

void print(const vec3& u) {
    std::cout << "[" << u.data.x << ", " << u.data.y << ", " << u.data.z << "]" << std::endl;
}

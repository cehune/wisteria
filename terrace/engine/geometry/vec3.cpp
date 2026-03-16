//
//  vec3.cpp
//  terrace
//
//  Created by celine on 2026-03-15.
//

#include "vec3.hpp"

// Default constructor: initializes to 0,0,0,0
vec3::vec3() : data((simd_float4){0, 0, 0, 0}) {}

// Initialize from raw SIMD type
vec3::vec3(simd_float4 v) : data(v) {}

// Magnitude (Length)
float vec3::magnitude() const {
    return simd_length(data);
}

// Subscript operator
float vec3::operator[](int axis) const {
    return data[axis];
}

// Unary minus (Inverse direction)
vec3 vec3::operator-() const {
    return vec3(-data);
}

// Assignment Addition
vec3& vec3::operator+=(const vec3& v) {
    data += v.data;
    return *this;
}

// Assignment Multiplication
vec3& vec3::operator*=(float t) {
    data *= t;
    return *this;
}

// Assignment Division
vec3& vec3::operator/=(float t) {
    data *= (1.0f / t);
    return *this;
}

// Equality
bool vec3::operator==(const vec3& u) {
    return simd_all(data == u.data);
}

// Inequality
bool vec3::operator!=(const vec3& u) {
    return !(*this == u);
}

// Normalization (Unit Vector)
vec3 vec3::normal_of() const {
    return vec3(simd_normalize(data));
}

// Check if vector is very small (useful for avoiding division by zero)
bool vec3::near_zero() const {
    const float s = 1e-8;
    return (std::fabs(data.x) < s) && (std::fabs(data.y) < s) && (std::fabs(data.z) < s);
}

// Find the axis with the largest value
int vec3::max_dimen() const {
    if (data.x > data.y && data.x > data.z) return 0;
    if (data.y > data.z) return 1;
    return 2;
}

//
//  vec3.hpp
//  terrace
//
//  Created by celine on 2026-03-15.
//

#include <simd/simd.h>
#include <cmath>
#include <simd/simd.h>
#pragma once

class vec3 {
public:
    // x, y, z, w
    simd_float4 data;
    
    vec3();
    vec3(simd_float4 v);
    
    float magnitude() const;
    float operator[](int axis) const;
    
    vec3 operator-() const;
    vec3& operator+=(const vec3& v);
    vec3& operator*=(float t);
    vec3& operator/=(float t);

    bool operator==(const vec3& u);
    bool operator!=(const vec3& u);

    vec3 normal_of() const;
    bool near_zero() const;
    int max_dimen() const;
};

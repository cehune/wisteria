//
//  GeometryUtils.hpp
//  wisteria
//
//  Created by celine on 2026-03-15.
//
#pragma once
#include <simd/simd.h>
#include "Config.hpp"
 
namespace Geometry {
 
    Mat4 perspective(float fovY, float aspect, float near, float far);
    Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up);
    Mat4 translate(Vec3 t);
    Mat4 scale(Vec3 s);
    Mat4 rotateX(float angle);
    Mat4 rotateY(float angle);
    Mat4 rotateZ(float angle);
 
    // Sampling
    Vec3 randomUnitVector();
    Vec3 randomOnHemisphere(Vec3 normal);
 
    // Debug
    void print(Vec3 v);
    void print(Vec4 v);
    void print(Mat4 m);
 
}

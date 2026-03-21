//
//  utils.cpp
//  terrace
//
//  Created by celine on 2026-03-15.
//

#include "GeometryUtils.hpp"
#include "Config.hpp"
#include <iostream>
#include <random>
#include <cmath>
 
namespace Geometry {
  
    Mat4 perspective(float fovY, float aspect, float near, float far) {
        float y = 1.0f / std::tan(fovY * 0.5f);
        float x = y / aspect;
        float z = far / (near - far);
        float w = (near * far) / (near - far);

        return Mat4{
            simd_make_float4(x,  0,  0,  0),
            simd_make_float4(0,  y,  0,  0),
            simd_make_float4(0,  0,  z, -1),
            simd_make_float4(0,  0,  w,  0)
        };
    }

    Mat4 lookAt(Vec3 eye, Vec3 center, Vec3 up) {
        Vec3 z = simd_normalize(eye - center);      // forward
        Vec3 x = simd_normalize(simd_cross(up, z)); // right
        Vec3 y = simd_cross(z, x);                  // up

        return Mat4{
            simd_make_float4(x[0], y[0], z[0], 0),
            simd_make_float4(x[1], y[1], z[1], 0),
            simd_make_float4(x[2], y[2], z[2], 0),
            simd_make_float4(-simd_dot(x, eye), -simd_dot(y, eye), -simd_dot(z, eye), 1)
        };
    }
 
    Mat4 translate(Vec3 t) {
        return Mat4 {
            simd_make_float4(1,    0,    0,    0), 
            simd_make_float4(0,    1,    0,    0), 
            simd_make_float4(0,    0,    1,    0), 
            simd_make_float4(t[0], t[1], t[2], 1)
        };
    }
 
    Mat4 scale(Vec3 s) {
        return Mat4{
            simd_make_float4(s[0], 0,    0,    0), 
            simd_make_float4(0,    s[1], 0,    0), 
            simd_make_float4(0,    0,    s[2], 0), 
            simd_make_float4(0,    0,    0,    1)
        };
    }
 
    Mat4 rotateX(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);

        return Mat4{
            simd_make_float4(1,  0,  0,  0), 
            simd_make_float4(0,  c,  s,  0), 
            simd_make_float4(0,  -s,  c,  0), 
            simd_make_float4(0,  0,  0,  1)  
        };
    }
 
    Mat4 rotateY(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);

        return Mat4{
            simd_make_float4(c,  0,  -s, 0), 
            simd_make_float4(0,  1,  0,  0), 
            simd_make_float4(s,  0,  c,  0), 
            simd_make_float4(0,  0,  0,  1)  
        };
    }
 
    Mat4 rotateZ(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);

        return Mat4{
            simd_make_float4(c,  s,  0,  0), 
            simd_make_float4(-s, c,  0,  0), 
            simd_make_float4(0,  0,  1,  0), 
            simd_make_float4(0,  0,  0,  1)  
        };
    }
 
    // ── Sampling ─────────────────────────────────────────────────
 
    Vec3 randomUnitVector() {
        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::mt19937 gen(seed);
        std::uniform_real_distribution<float> distrib(-1.0f, 1.0f);

        float x = distrib(gen);
        float y = distrib(gen);
        float z = distrib(gen);

        float norm = std::sqrt(x*x + y*y + z*z);
        return Vec3{ x / norm, y / norm, z / norm };
    }

    Vec3 randomOnHemisphere(Vec3 normal) {
        Vec3 v = randomUnitVector();
        // If the vector points away from the normal, flip it
        if (simd_dot(v, normal) < 0.0f) {
            v = -v;
        }
        return v;
    }
 
    // ── Debug ─────────────────────────────────────────────────────
 
    void print(Vec3 v) {
        std::cout << "(x, y, z): (" << v[0] << ", " << v[1] << ", " << v[2] << ")\n";
    }
 
    void print(Vec4 v) {
        std::cout << "(x, y, z, w): (" << v[0] << ", " << v[1] << ", " << v[2] << ", " << v[3] << ")\n";
    }
 
    void print(Mat4 m) {
        std::cout << "Matrix 4x4:\n";
        for (int row = 0; row < 4; ++row) {
            std::cout << "  ";
            for (int col = 0; col < 4; ++col) {
                std::cout << m.columns[col][row] << " ";
            }
            std::cout << "\n";
        }
    }
 
}

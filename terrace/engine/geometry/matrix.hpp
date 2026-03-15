//
//  matrix.hpp
//  terrace
//
//  Created by celine on 2026-03-15.
//
// 
//

#include <simd/simd.h>
#pragma once

using namespace simd;

class matrix {
public:
    simd_float4x4 data;
    
    matrix(); // creates identity matrix by default
    matrix(float x); // fills all with the same value
    matrix(const matrix& mat);
    matrix(const simd_float4x4& d);
    matrix operator=(const matrix& m);
    
    // just operations
    matrix operator+(const matrix& m); // matrix + matrix
    matrix operator*(const matrix& m); // matrix * matrix
    matrix operator*(float x); // matrix * float
    matrix operator/(float x); //matrix / float
    
    // reassignment for the current matrix
    matrix& operator+=(const matrix& m); // matrix += matrix
    matrix& operator*=(const matrix& m); // matrix *= matrix
    matrix& operator*=(float x);
    matrix& operator/=(float x); // matrix /= float

    
    matrix transpose() const;
    matrix inverse() const;
    float determinant() const;
};


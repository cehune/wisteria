//
//  matrix.cpp
//  terrace
//
//  Created by celine on 2026-03-15.
//
#include "matrix.hpp"
// creates identity matrix by default
matrix::matrix() {
    this->data = matrix_identity_float4x4;
}

// fills all with the same value
matrix::matrix(float x) {
    simd_float4 col = {x, x, x, x};
    this->data = simd_matrix(col, col, col, col);
}

matrix::matrix(const simd_float4x4& d): data(d) {}

matrix::matrix(const matrix& mat): data(mat.data) {}


/* Operators */
// addition
matrix matrix::operator+(const matrix& m) {
    return matrix(this->data + m.data);
}
// mat x mat multiplication
matrix matrix::operator*(const matrix& m) {
    return matrix(simd_mul(this->data, m.data));
}

// mat x scalar multiplication
matrix matrix::operator*(float x) {
    return matrix(this->data * x);
}

// mat x scalar division
matrix matrix::operator/(float x) {
    float inv = 1.0f / x; // this is faster than a strict divide
    return matrix(this->data * inv);
}

/* reassignment for the current matrix */
// mat x mat addition
matrix& matrix::operator+=(const matrix& m) {
    this->data = this->data + m.data;
    return *this;
}
// mat x mat multiplication
matrix& matrix::operator*=(const matrix& m) {
    this->data = simd_mul(this->data, m.data);
    return *this;
}
// mat x scalar addition
matrix& matrix::operator*=(float x) {
    this->data = this->data * x;
    return *this;
}
// mat x scalar division
matrix& matrix::operator/=(float x) {
    float inv = 1.0f / x;
    this->data = this->data * inv;
    return *this;
}

// related matrices
matrix matrix::transpose() const {
    return simd_transpose(this->data);
}
matrix matrix::inverse() const {
    return simd_inverse(this->data);
}
float matrix::determinant() const {
    return simd_determinant(this->data);
}

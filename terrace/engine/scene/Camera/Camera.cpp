//
//  Camera.cpp
//  terrace
//
//  Created by celine on 2026-04-25.
//

#include "Camera.hpp"

Camera::Camera(): _orientation(simd_quaternion(0.0f, 0.0f, 0.0f, 1.0f)), position{0, 0, 0}
    , _fov(M_PI / 3.0f), _near(0.1f), _far(1000.0f) {}

simd_float4x4 Camera::viewMatrix() const {
    // Luckily simd internally can do the quaternion to rotation conversion
    // recall that view matrices essentially contain the forward, right, and up directions
    // based on the position, and look at. Quaternions simplify and are better for runtime.
    
    // quat to matrix is camera rotation in world space. to go from world to camera, we
    // need to transpose the quat matrix.
    auto R = simd_transpose(simd_matrix4x4(_orientation));
    auto T = matrix_identity_float4x4;
    T.columns[3] = simd_make_float4(-position.x, -position.y, -position.z, 1);
    return simd_mul(R, T);
}

simd_float4x4 Camera::projectionMatrix(float aspect) const {
    // holy grail link:
    /*https://scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix.html*/
    
    /*
     Perspective projection for Metal NDC: depth in [0,1], column-major
     */
    float ys = 1.0f / tanf(_fov * 0.5f);
    float xs = ys / aspect;
    float zs = _far / (_near - _far);
    
    return simd_float4x4{{
        {xs, 0,  0,         0},
        {0,  ys, 0,         0},
        {0,  0,  zs,       -1},
        {0,  0,  zs * _near, 0}
    }};
}

void Camera::rotate(float dyaw, float dpitch) {
    //
    simd_quatf yawQ   = simd_quaternion(dyaw,   simd_make_float3(0,1,0));
    simd_quatf pitchQ = simd_quaternion(dpitch,  right()); // current local right axis
    _orientation = simd_normalize(simd_mul(yawQ, simd_mul(pitchQ, _orientation)));
}

void Camera::moveRight(float dx)   { position += right()   * dx; }
void Camera::moveUp(float dy)      { position += up()      * dy; }
void Camera::moveForward(float dz) { position += forward() * dz; }



//
//  Camera.cpp
//  terrace
//
//  Created by celine on 2026-04-25.
//

#include "Camera.hpp"

simd_float4x4 Camera::viewMatrix(CameraState& state) const {
    // Luckily simd internally can do the quaternion to rotation conversion
    // recall that view matrices essentially contain the forward, right, and up directions
    // based on the position, and look at. Quaternions simplify and are better for runtime.
    
    // quat to matrix is camera rotation in world space. to go from world to camera, we
    // need to transpose the quat matrix.
    auto R = simd_transpose(simd_matrix4x4(state.orientation));
    auto T = matrix_identity_float4x4;
    T.columns[3] = simd_make_float4(-state.position.x, -state.position.y, -state.position.z, 1);
    return simd_mul(R, T);
}

simd_float4x4 Camera::projectionMatrix(CameraState& state, float aspect) const {
    // holy grail link:
    /*https://scratchapixel.com/lessons/3d-basic-rendering/perspective-and-orthographic-projection-matrix/building-basic-perspective-projection-matrix.html*/
    
    /*
     Perspective projection for Metal NDC: depth in [0,1], column-major
     */
    float ys = 1.0f / tanf(state.fov * 0.5f);
    float xs = ys / aspect;
    float zs = state.far / (state.near - state.far);
    
    return simd_float4x4{{
        {xs, 0,  0,         0},
        {0,  ys, 0,         0},
        {0,  0,  zs,       -1},
        {0,  0,  zs * state.near, 0}
    }};
}

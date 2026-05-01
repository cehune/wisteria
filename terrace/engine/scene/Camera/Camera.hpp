//
//  Camera.hpp
//  terrace
//
//  Created by celine on 2026-04-25.
//
#pragma once
#include "engine/geometry/Config.hpp"
struct CameraState {
    Vec3       position    = {0, 0, 0};
    simd_quatf orientation = simd_quaternion(0.f, 0.f, 0.f, 1.f);
    float      fov         = M_PI / 3.0f;
    float      near        = 0.1f;
    float      far         = 1000.0f;
};

class Camera { // mainly for the raster because we need to control it
public:
    Vec3 right(CameraState& state) const { return simd_act(state.orientation, Vec3{1, 0, 0});};
    Vec3 up(CameraState& state) const { return simd_act(state.orientation, Vec3{0, 1, 0});};
    Vec3 forward(CameraState& state) const { return simd_act(state.orientation, Vec3{0, 0, -1});};
    simd_float4x4 viewMatrix(CameraState& state) const;
    simd_float4x4 projectionMatrix(CameraState& state, float aspect) const;
};

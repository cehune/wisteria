//
//  OrbitController.cpp
//  terrace
//
//  Created by celine on 2026-04-27.
//

#pragma once
#include "CameraController.hpp"
#include "simd/simd.h"

class OrbitController : public CameraController {
public:
    CameraState update(const CameraState& current, float dt) override;
    void onMouseDrag(float dx, float dy) override;
    void onScroll(float delta) override;
    void onKey(int key, bool pressed) override;

private:
    Vec3  _target      = {0, 0, 0};
    float _radius      = 5.0f;
    float _yaw         = 0.0f;
    float _pitch       = 0.3f;
    float _sensitivity = 0.005f;
    float _zoomSpeed   = 0.3f;
};

inline CameraState OrbitController::update(const CameraState& current, float dt) {
    CameraState next = current;

    float cosPitch = cosf(_pitch);
    float sinPitch = sinf(_pitch);
                          
    float cosYaw = cosf(_yaw);
    float sinYaw = sinf(_yaw);
    
    // Draw a box with a r vector going to opposite corners.
    // the direction angles are derived with yaw defined as 0 from the z axis.
    Vec3 dir      = {cosPitch * sinYaw, sinPitch, cosPitch * cosYaw};
    next.position = _target + _radius * dir;

    // dir is already unit vector for the position of camera to target
    Vec3 right        = simd_normalize(simd_cross(-dir, {0, 1, 0}));
    Vec3 up           = simd_cross(right, -dir);
                          
    next.orientation  = quatFromAxes(right, up, dir);
    return next;
}

inline void OrbitController::onMouseDrag(float dx, float dy) {
    _yaw  += dx * _sensitivity;
    _pitch = std::clamp(_pitch + dy * _sensitivity, -M_PI_2 + 0.01f, M_PI_2 - 0.01f);
}

inline void OrbitController::onScroll(float delta) {
    _radius = std::max(0.1f, _radius - delta * _zoomSpeed);
}

inline void OrbitController::onKey(int key, bool pressed) {
    // stub — implement pan and reset here after mesh instancing
}


//
//  OrbitController.hpp
//  terrace
//
//  Created by celine on 2026-04-27.
//

#pragma once
#include "CameraController.hpp"
#include <simd/simd.h>
#include <algorithm>
#include <cmath>

class OrbitController : public CameraController {
public:
    CameraState update(const CameraState& current, float dt) override;
    void onMouseDrag(float dx, float dy) override;
    void onScroll(float delta) override;

private:
    simd_quatf quatFromAxes(const simd_float3& right, 
                            const simd_float3& up, 
                            const simd_float3& forward);

    simd_float3  _target      = {0, 0, 0};
    float _radius      = 20.0f;
    float _yaw         = 0.0f;
    float _pitch       = -0.3f;
    float _sensitivity = 0.005f;
    float _zoomSpeed   = 0.3f;
};

inline simd_quatf OrbitController::quatFromAxes(const simd_float3& right,
                               const simd_float3& up,
                               const simd_float3& forward)
{
    simd_float3x3 m = {right, up, forward};
    return simd_quaternion(m);
}

inline CameraState OrbitController::update(const CameraState& current, float dt) {
    CameraState next = current;
    float cosPitch = cosf(_pitch);
    float sinPitch = sinf(_pitch);
                          
    float cosYaw = cosf(_yaw);
    float sinYaw = sinf(_yaw);
    
    // Draw a box with a r vector going to opposite corners.
    // the direction angles are derived with yaw defined as 0 from the z axis.
    simd_float3 dir      = {cosPitch * sinYaw, sinPitch, cosPitch * cosYaw};
    next.position = _target + _radius * dir;

    // dir is already unit vector for the position of camera to target
    simd_float3 worldUp = {0, 1, 0}; // prevent exploding when dir near up
    if (fabs(simd_dot(dir, worldUp)) > 0.99999f) {
        worldUp = {1, 0, 0}; // avoid degeneracy
    }

    simd_float3 right = simd_normalize(simd_cross(-dir, worldUp));
    simd_float3 up    = simd_cross(right, -dir);
                          
    next.orientation  = quatFromAxes(right, up, dir);
    return next;
}

inline void OrbitController::onMouseDrag(float dx, float dy) {
    _yaw  += dx * _sensitivity;
    _pitch = std::min(std::max(_pitch + dy * _sensitivity, (float)-M_PI_2 + 0.1f), (float)M_PI_2 - 0.1f);
}

inline void OrbitController::onScroll(float delta) {
    _radius = std::max(0.1f, _radius - delta * _zoomSpeed);
}


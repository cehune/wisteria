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

// TODO remove state info from the camera class, should just have operations for matrices

class Camera { // mainly for the raster because we need to control it
private:
    float _fov, _near, _far;
    simd_quatf _orientation;
    
public:
    Camera();
    Vec3 position;
    
    Vec3 right() const { return simd_act(_orientation, Vec3{1, 0, 0});};
    Vec3 up() const { return simd_act(_orientation, Vec3{0, 1, 0});};
    Vec3 forward() const { return simd_act(_orientation, Vec3{0, 0, -1});};
    
    simd_float4x4 viewMatrix() const;
    simd_float4x4 projectionMatrix(float aspect) const;
    
    void rotate(float dyaw, float dpitch);
    void moveRight(float dx);
    void moveUp(float dy);
    void moveForward(float dz);
    
    float getFov()  const { return _fov; }
    float getNear() const { return _near; }
    float getFar()  const { return _far; }
    void setFov(float f)   { _fov = f; }
    void setClip(float n, float f) { _near = n; _far = f; }
};

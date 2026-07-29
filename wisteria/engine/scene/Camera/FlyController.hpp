//
//  FlyController.hpp
//  wisteria
//
//  WASD + mouse-look fly camera for scene design.
//
#pragma once
#include "CameraController.hpp"
#include <simd/simd.h>
#include <algorithm>
#include <cmath>

class FlyController : public CameraController {
public:
    bool update(CameraState& state, float dt) override;
    void onMouseDrag(float dx, float dy) override;
    void onScroll(float delta) override;
    void onAction(CameraAction action, bool pressed) override;

    // Input gains, tunable at runtime (e.g. from a settings GUI later).
    void  setMoveSpeed(float unitsPerSecond)        { _speed = unitsPerSecond; }
    float moveSpeed() const                         { return _speed; }
    void  setLookSensitivity(float radiansPerPixel) { _sensitivity = radiansPerPixel; }
    float lookSensitivity() const                   { return _sensitivity; }

private:
    simd_float3 _position    = {0.0f, 2.0f, 20.0f};
    float       _yaw         = 0.0f;    // radians, around world up (+Y)
    float       _pitch       = 0.0f;    // radians, around camera right
    float       _speed       = 10.0f;   // units / second (tunable via setMoveSpeed)
    float       _sensitivity = 0.005f;  // radians / pixel

    bool _fwd = false, _back = false, _left = false, _right = false,
         _up = false, _down = false;

    // Starts true so the first update seeds the camera from whatever state the
    // owner supplies, and is set by anything that actually moves the camera.
    bool _dirty = true;
    bool _seeded = false;
};

inline bool FlyController::update(CameraState& state, float dt) {
    // Adopt the caller's starting view once, so the owner of the camera picks
    // the initial framing rather than this controller's defaults.
    if (!_seeded) {
        _position = state.position;
        _seeded   = true;
    }

    const bool moving = _fwd || _back || _left || _right || _up || _down;
    if (!moving && !_dirty) return false;   // nothing changed; don't reset accumulation

    float cosPitch = cosf(_pitch);
    float sinPitch = sinf(_pitch);
    float cosYaw   = cosf(_yaw);
    float sinYaw   = sinf(_yaw);

    // Forward (look) direction. yaw=pitch=0 (0,0,-1).
    simd_float3 dir = simd_normalize(
        simd_float3{cosPitch * sinYaw, sinPitch, cosPitch * -cosYaw});

    // The pitch clamp in onMouseDrag keeps |dir . worldUp| < 1, which is what
    // prevents this cross product from degenerating. Do not loosen that clamp
    // without adding a guard here.
    const simd_float3 worldUp = {0.0f, 1.0f, 0.0f};
    const simd_float3 right   = simd_normalize(simd_cross(dir, worldUp));
    const simd_float3 up      = simd_cross(right, dir);

    // Integrate position from held keys (camera-relative; vertical is world up).
    simd_float3 vel = {0.0f, 0.0f, 0.0f};
    if (_fwd)   vel += dir;
    if (_back)  vel -= dir;
    if (_right) vel += right;
    if (_left)  vel -= right;
    if (_up)    vel += worldUp;
    if (_down)  vel -= worldUp;
    if (simd_length(vel) > 1e-5f) {
        _position += simd_normalize(vel) * (_speed * dt);
    }
    state.position = _position;

    // Orientation columns [right, up, +Z]; +Z = -dir
    simd_float3x3 basis = { right, up, -dir };
    state.orientation = simd_quaternion(basis);

    _dirty = false;
    return true;
}

inline void FlyController::onMouseDrag(float dx, float dy) {
    _dirty = true;
    _yaw += dx * _sensitivity;
    _pitch = std::clamp(_pitch + dy * _sensitivity,
                        -(float)M_PI_2 + 0.05f, (float)M_PI_2 - 0.05f);
}

inline void FlyController::onScroll(float delta) {
    // Scroll tunes fly speed — it does not move the camera, so this
    // deliberately does not mark the state dirty.
    _speed = std::clamp(_speed + delta * 0.5f, 0.5f, 100.0f);
}

inline void FlyController::onAction(CameraAction action, bool pressed) {
    _dirty = true;   // key down starts motion; key up ends it, both are changes
    switch (action) {
        case CameraAction::Forward: _fwd   = pressed; break;
        case CameraAction::Back:    _back  = pressed; break;
        case CameraAction::Left:    _left  = pressed; break;
        case CameraAction::Right:   _right = pressed; break;
        case CameraAction::Up:      _up    = pressed; break;
        case CameraAction::Down:    _down  = pressed; break;
    }
}

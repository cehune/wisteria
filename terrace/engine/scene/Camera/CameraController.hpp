//
//  CameraController.hpp
//  terrace
//
//  Created by celine on 2026-04-27.
//

#pragma once
#include "Camera.hpp"
#include "engine/geometry/Config.hpp"
#include "CameraController.hpp"

// Semantic camera movement actions. Platform code maps device input
enum class CameraAction { Forward, Back, Left, Right, Up, Down };

class CameraController {
public:
    virtual ~CameraController() = default;
    virtual CameraState update(const CameraState& current, float dt) = 0;
    virtual void onMouseDrag(float dx, float dy) = 0;
    virtual void onScroll(float delta) = 0;
    // Default controllers don't translate movement (e.g. orbit) with keys
    virtual void onAction(CameraAction action, bool pressed) {}
};

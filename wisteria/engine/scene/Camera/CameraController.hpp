//
//  CameraController.hpp
//  wisteria
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

    // Writes the updated camera into `state` and returns whether it actually changed it.
    // Bool is necessary so progressive backends don't have to reset their accumulation
    // if the camera doesn't move
    //
    // The first call adopts `state` as the starting point
    virtual bool update(CameraState& state, float dt) = 0;

    virtual void onMouseDrag(float dx, float dy) = 0;
    virtual void onScroll(float delta) = 0;
    // Default controllers don't translate movement (e.g. orbit) with keys
    virtual void onAction(CameraAction action, bool pressed) {}
};

//
//  CameraUniforms.hpp
//  terrace
//
//  Created by celine on 2026-04-21.
//
#pragma once
#include "Core.hpp"

struct CameraUniformsPT {
    Vec3 origin;
    Vec3 forward;
    Vec3 up;
    Vec3 right;
    float fov;
};

// We don't actually need the model matrix, thats stored per mesh
struct CameraUniformsRaster {
    Vec4 view;
    Vec4 projection;
    Vec4 viewProjection;
    Vec3 worldPosition;
    float pad; // just for extra padding alignment
};

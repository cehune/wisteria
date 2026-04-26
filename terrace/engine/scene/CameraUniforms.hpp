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
    simd_float4x4 viewProjection;
};

//
//  CameraUniforms.hpp
//  terrace
//
//  Created by celine on 2026-04-21.
//
#include "Core.hpp"

struct CameraUniformsPT {
    Vec3 origin;
    Vec3 forward;
    Vec3 up;
    Vec3 right;
    float fov;
};

struct CameraUniformsRaster {
    Vec4 model;
    Vec4 view;
    Vec4 projection;
};

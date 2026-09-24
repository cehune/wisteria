//
//  SceneEnvironment.hpp
//  wisteria
//
//  Created by celine on 2026-09-24.
//  Why a scene env? rasterization is image based lighting and i do think the 
//  main other option is by having a special light source which would be unoptimal...
//  Don't want to bloat the mem too much :)
//
#pragma once

struct SceneEnvironment {
    float radiance = 0.0f;
};

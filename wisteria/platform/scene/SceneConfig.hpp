//
//  SceneConfig.hpp
//  wisteria
//
//  Created by celine on 2026-09-24.
//  What world to load -- separate from RenderConfig (how to render it: width, height,
//  backend, spp). Just the OBJ path for now; grows the same way SceneEnvironment does,
//  as something else actually needs to live next to it.
//
#pragma once
#include <string>

struct SceneConfig {
    std::string path;   // empty -> bundled cornell_box.obj
};

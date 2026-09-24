//
//  RenderConfig.hpp
//  wisteria
//
//  Created by celine on 2026-07-27.
//
#pragma once
#include <cstdint>
#include <string>


enum class BackendType { Raster, PathTracer };

// params that shouldn't change if we load a new model
struct RenderConfig {
    BackendType backend   = BackendType::Raster;   // edit-first
    uint32_t    width     = 800;
    uint32_t    height    = 600;

    // Offline rendering
    bool        headless      = false;
    uint32_t    targetSamples = 512;
    std::string outPath;
};

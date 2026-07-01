//
//  Core.hpp
//  wisteria
//
//  Created by celine on 2026-03-19.
//
//  Stores all class structs that are diff
#pragma once
#include <iostream>
#include <cstdint>
#include "engine/geometry/Config.hpp"
#include "platform/shaders/SharedTypes.h"   // struct Vertex (shared CPU/GPU ABI)

using PipelineID = uint32_t;

inline void printVertex(const Vertex& v) {
    std::cout << "position: " << v.position.x << ", " << v.position.y << ", " << v.position.z << "\n";
    std::cout << "normal:   " << v.normal.x   << ", " << v.normal.y   << ", " << v.normal.z   << "\n";
    std::cout << "uv:       " << v.uv.x       << ", " << v.uv.y       << "\n";
}

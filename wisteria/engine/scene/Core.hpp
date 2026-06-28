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

using PipelineID = uint32_t;

// vertexes
struct Vertex {
    Vec4 position;
    Vec4 normal;
    Vec2 uv;
    Vec4 tangent;
    Vec4 color;
};

inline void printVertex(const Vertex& v) {
    std::cout << "position: " << v.position.x << ", " << v.position.y << ", " << v.position.z << "\n";
    std::cout << "normal:   " << v.normal.x   << ", " << v.normal.y   << ", " << v.normal.z   << "\n";
    std::cout << "uv:       " << v.uv.x       << ", " << v.uv.y       << "\n";
}

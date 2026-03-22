//
//  Core.hpp
//  terrace
//
//  Created by celine on 2026-03-19.
//
//  Stores all class structs that are diff
#pragma once
#include <cstdint>
#include "engine/geometry/Config.hpp"

using PipelineID = uint32_t;       // ← just a uint32_t

// vertexes
struct Vertex {
    Vec4 position;
    Vec4 normal;
    Vec2 uv;
    Vec4 tangent;
    Vec4 color;
};



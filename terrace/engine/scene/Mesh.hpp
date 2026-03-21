//
//  Mesh.hpp
//  terrace
//
//  Created by celine on 2026-03-19.
//

#pragma once
#include "Core.hpp"
#include "engine/renderer/PipelineKey.hpp"

struct Mesh {
    uint8_t index; // in scene meshes vector
    std::string name;
    
    uint32_t vertexOffset;
    uint32_t indexOffset;
    uint32_t numTriangles;
    Vec4 localBoundsMin;
    Vec4 localBoundsMax;
    std::vector<int> meshInstanceIndexes;
};

struct MeshInstance {
    uint8_t meshIndex; // in scene meshes vector
    uint8_t index; // in scene meshInstance vector
    uint32_t materialID;
    simd_float4x4 transform;
    PipelineID pipelineID; // index of pipelineID in renderer
    Vec4 boundsMin;
    Vec4 boundsMax;
};

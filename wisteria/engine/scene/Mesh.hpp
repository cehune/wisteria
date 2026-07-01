//
//  Mesh.hpp
//  wisteria
//
//  Created by celine on 2026-03-19.
//

#pragma once
#include "Core.hpp"
#include "engine/renderer/PipelineKey.hpp"

struct Mesh {
    unsigned long index; // in scene meshes vector
    std::string name;

    // Offsets into the SceneGeometryPool megabuffers.
    // vertexOffset/indexOffset are measured in elements (not bytes).
    // Indices stored in the pool are GLOBAL (already rebased to the mega vertex buffer).
    uint32_t vertexOffset;
    uint32_t indexOffset;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t numTriangles;

    Vec4 localBoundsMin;
    Vec4 localBoundsMax;
    std::vector<int> meshInstanceIndexes;
};

struct MeshInstance {
    uint32_t meshIndex; // in scene meshes vector
    uint32_t index;     // in scene meshInstance vector
    uint32_t materialID;
    simd_float4x4 transform;
    PipelineID pipelineID; // index of pipelineID in renderer
    Vec4 boundsMin;
    Vec4 boundsMax;

    // Mirrored from Mesh so an instance is self-sufficient on the GPU
    // (e.g. for a per-instance descriptor buffer the path tracer can read).
    uint32_t vertexOffset;
    uint32_t indexOffset;
    uint32_t indexCount;
};

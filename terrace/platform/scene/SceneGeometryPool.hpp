//
//  SceneGeometryPool.hpp
//  terrace
//
//  Created by celine on 2026-03-20.
#pragma once
#include <vector>
#include <Metal/Metal.hpp>
#include "engine/scene/Mesh.hpp"
#include "engine/scene/Core.hpp" // 6429

class SceneGeometryPool {
public:
    void uploadMesh(Mesh& mesh,
                    const std::vector<Vertex>& verts,
                    const std::vector<uint32_t>& indices,
                    MTL::Device* device);
    std::pair<Vec4, Vec4> calculateBounds(const std::vector<Vertex>& verts);

private:
    // eventually change these to mega buffer for optimization
    std::vector<MTL::Buffer*> _vertexBuffers;
    std::vector<MTL::Buffer*> _indexBuffers;
    uint32_t _meshCount = 0;
};

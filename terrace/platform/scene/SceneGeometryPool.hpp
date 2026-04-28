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
#include "engine/geometry/MeshLoader.hpp"
#include "IGeometryPool.hpp"

class SceneGeometryPool: public IGeometryPool {
public:
    SceneGeometryPool();
    void uploadMesh(Mesh& mesh,
                    const std::vector<Vertex>& verts,
                    const std::vector<uint32_t>& indices,
                    MTL::Device* device) override;
    
    void uploadMeshFile(Mesh& mesh, const std::string& meshPath, MTL::Device* device) override;
    std::pair<Vec4, Vec4> calculateBounds(const std::vector<Vertex>& verts);
    MTL::Buffer* vertexBufferFor(const Mesh& mesh) override;
    MTL::Buffer* indexBufferFor(const Mesh& mesh) override;
    MTL::Buffer* mergeVertexBuffer(MTL::Device* device) override;
    MTL::Buffer* mergeIndexBuffer(MTL::Device* device) override;

private:
    // eventually change these to mega buffer for optimization
    std::vector<MTL::Buffer*> _vertexBuffers;
    std::vector<MTL::Buffer*> _indexBuffers;
    uint32_t _meshCount = 0;
    MeshLoader                _meshLoader;
};

//
//  MockGeometryPool.hpp
//  wisteria
//
//  Created by celine on 2026-04-27.
//
#pragma once
#include "platform/scene/IGeometryPool.hpp"

class MockGeometryPool : public IGeometryPool {
public:
    // Mirror SceneGeometryPool's bookkeeping (offsets/counts) without any Metal,
    // so Scene's mesh/instance tracking can be tested against realistic values.
    void uploadMesh(Mesh& mesh,
                    const std::vector<Vertex>& verts,
                    const std::vector<uint32_t>& indices,
                    MTL::Device* device) override {
        mesh.vertexOffset = (uint32_t)_stagedVerts;
        mesh.indexOffset  = (uint32_t)_stagedIndices;
        mesh.vertexCount  = (uint32_t)verts.size();
        mesh.indexCount   = (uint32_t)indices.size();
        mesh.numTriangles = (uint32_t)(indices.size() / 3);
        mesh.localBoundsMin = {0, 0, 0, 0};
        mesh.localBoundsMax = {0, 0, 0, 0};
        _stagedVerts   += verts.size();
        _stagedIndices += indices.size();
        _uploadCount++;
    }

    MTL::Buffer* vertexBuffer() override { return nullptr; }
    MTL::Buffer* indexBuffer()  override { return nullptr; }
    void finalize() override {}

    int    _uploadCount   = 0;
    size_t _stagedVerts   = 0;
    size_t _stagedIndices = 0;
};

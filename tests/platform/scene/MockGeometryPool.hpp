//
//  MockGeometryPool.hpp
//  terrace
//
//  Created by celine on 2026-04-27.
//
#pragma once
#include "platform/scene/IGeometryPool.hpp"

class MockGeometryPool : public IGeometryPool {
public:
    void uploadMesh(Mesh& mesh,
                           const std::vector<Vertex>& verts,
                           const std::vector<uint32_t>& indices,
                    MTL::Device* device) override {
        return;
    }
    void uploadMeshFile(Mesh& mesh, const std::string& path, MTL::Device* device) override {
        // just populate bounds with dummy data, no Metal
        mesh.localBoundsMin = {-1, -1, -1, 0};
        mesh.localBoundsMax = { 1,  1,  1, 0};
        mesh.numTriangles   = 1;
        _uploadCount++;
    }

    MTL::Buffer* vertexBufferFor(const Mesh& mesh) override { return nullptr; }
    MTL::Buffer* indexBufferFor(const Mesh& mesh)  override { return nullptr; }
    
    MTL::Buffer* mergeVertexBuffer(MTL::Device* device) override {
        return nullptr;
    }
    MTL::Buffer* mergeIndexBuffer(MTL::Device* device) override{
        return nullptr;
    }

    int _uploadCount = 0;
};

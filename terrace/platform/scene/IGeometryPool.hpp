//
//  IGeometryPool.hpp
//  terrace
//
//  Created by celine on 2026-04-27.
//
// Interface for pool for testing and dependency injection.
#pragma once
class IGeometryPool {
public:
    virtual ~IGeometryPool() = default;
    virtual void uploadMesh(Mesh& mesh,
                            const std::vector<Vertex>& verts,
                            const std::vector<uint32_t>& indices,
                            MTL::Device* device) = 0;
    virtual void uploadMeshFile(Mesh& mesh, const std::string& path,
                                MTL::Device* device) = 0;
    virtual MTL::Buffer* vertexBufferFor(const Mesh& mesh) = 0;
    virtual MTL::Buffer* indexBufferFor(const Mesh& mesh) = 0;
    virtual MTL::Buffer* mergeVertexBuffer(MTL::Device* device) = 0;
    virtual MTL::Buffer* mergeIndexBuffer(MTL::Device* device) = 0;
};


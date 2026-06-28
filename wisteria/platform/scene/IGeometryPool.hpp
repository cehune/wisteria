//
//  IGeometryPool.hpp
//  wisteria
//
//  Created by celine on 2026-04-27.
//
// Interface for pool for testing and dependency injection.
#pragma once
class IGeometryPool {
public:
    virtual ~IGeometryPool() = default;

    // Append a mesh's geometry into the pool's staging buffers.
    // Record all mesh instance related stuff
    virtual void uploadMesh(Mesh& mesh,
                            const std::vector<Vertex>& verts,
                            const std::vector<uint32_t>& indices,
                            MTL::Device* device) = 0;
    virtual void uploadMeshFile(Mesh& mesh, const std::string& path,
                                MTL::Device* device) = 0;

    // just meant to be accessors. The pool may lazily rebuild if more meshes added dynamically, 
    // so never assume yours is always correct, so these will rebuild it and then return
    virtual MTL::Buffer* vertexBuffer() = 0;
    virtual MTL::Buffer* indexBuffer() = 0;

    // build trigger. The 
    virtual void finalize() = 0;
};

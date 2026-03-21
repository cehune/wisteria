//
//  SceneGeometryPool.cpp
//  terrace
//
//  Created by celine on 2026-03-21.
//

#include "SceneGeometryPool.hpp"

void SceneGeometryPool::uploadMesh(Mesh& mesh,
                                   const std::vector<Vertex>& verts,
                                   const std::vector<uint32_t>& indices,
                                   MTL::Device* device) {
    /* Saves one buffer per mesh, TODO: change to megabuffer*/
    
    size_t vertSize = verts.size() * sizeof(Vertex);
    size_t indiceSize = indices.size() * sizeof(uint32_t);
    
    // generate buffers
    MTL::Buffer* vertBuf = device->newBuffer(verts.data(), vertSize, MTL::ResourceStorageModeShared);
    MTL::Buffer* indiceBuf = device->newBuffer(indices.data(), indiceSize, MTL::ResourceStorageModeShared);
    
    // append buffers and set the meshes to be aware
    _vertexBuffers.push_back(vertBuf);
    _indexBuffers.push_back(indiceBuf);
    
    // these offsets will now set to the index of the respective buffers in the buffer storage arrays
    mesh.vertexOffset = _meshCount;
    mesh.indexOffset = _meshCount;
    mesh.numTriangles = static_cast<int>(indices.size() / 3);
    
    auto [min, max] = calculateBounds(verts);
    mesh.localBoundsMin = min;
    mesh.localBoundsMax  = max;
    _meshCount += 1;
}

/* Calculates bounds of a mesh or vector of vertices. Vec4 because it returns the min and max POSITION*/
std::pair<Vec4, Vec4> SceneGeometryPool::calculateBounds(const std::vector<Vertex>& verts) {
    Vec4 boundsMin{ FLT_MAX,  FLT_MAX,  FLT_MAX, 0};
    Vec4 boundsMax{-FLT_MAX, -FLT_MAX, -FLT_MAX, 0};
    
    for (const Vertex vert : verts) {
        boundsMin[0] = std::min(boundsMin[0], vert.position[0]);
        boundsMin[1] = std::min(boundsMin[1], vert.position[1]);
        boundsMin[2] = std::min(boundsMin[2], vert.position[2]);
        
        boundsMax[0] = std::max(boundsMax[0], vert.position[0]);
        boundsMax[1] = std::max(boundsMax[1], vert.position[1]);
        boundsMax[2] = std::max(boundsMax[2], vert.position[2]);
    }
    return {boundsMin, boundsMax};
}

//
//  SceneGeometryPool.cpp
//  terrace
//
//  Created by celine on 2026-03-21.
//

#include "SceneGeometryPool.hpp"

SceneGeometryPool::SceneGeometryPool() {
    _meshLoader = MeshLoader();
}

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

void SceneGeometryPool::uploadMeshFile(Mesh& mesh, const std::string& meshPath, MTL::Device* device) {
    /* Saves one buffer per mesh, TODO: change to megabuffer*/
    std::vector<Vertex> verts;
    std::vector<uint32_t> indices;
    
    _meshLoader.loadObjMesh(meshPath, verts, indices);
    
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
    // TODO: these will be different for mega buffer
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
    
    for (auto& vert: verts) {
        boundsMin[0] = std::min(boundsMin[0], vert.position[0]);
        boundsMin[1] = std::min(boundsMin[1], vert.position[1]);
        boundsMin[2] = std::min(boundsMin[2], vert.position[2]);
        
        boundsMax[0] = std::max(boundsMax[0], vert.position[0]);
        boundsMax[1] = std::max(boundsMax[1], vert.position[1]);
        boundsMax[2] = std::max(boundsMax[2], vert.position[2]);
    }
    return {boundsMin, boundsMax};
}

MTL::Buffer* SceneGeometryPool::vertexBufferFor(const Mesh& mesh) {
    return _vertexBuffers[mesh.vertexOffset];
}
MTL::Buffer* SceneGeometryPool::indexBufferFor(const Mesh& mesh) {
    return _indexBuffers[mesh.indexOffset];
}

// SceneGeometryPool.cpp
MTL::Buffer* SceneGeometryPool::mergeVertexBuffer(MTL::Device* device) {
    std::vector<Vertex> all;
    for (auto* buf : _vertexBuffers) {
        Vertex* ptr = static_cast<Vertex*>(buf->contents());
        size_t count = buf->length() / sizeof(Vertex);
        all.insert(all.end(), ptr, ptr + count);
    }
    return device->newBuffer(all.data(), all.size() * sizeof(Vertex), MTL::ResourceStorageModeShared);
}

MTL::Buffer* SceneGeometryPool::mergeIndexBuffer(MTL::Device* device) {
    std::vector<uint32_t> all;
    uint32_t vertexBase = 0;
    for (size_t i = 0; i < _indexBuffers.size(); i++) {
        uint32_t* ptr = static_cast<uint32_t*>(_indexBuffers[i]->contents());
        size_t count = _indexBuffers[i]->length() / sizeof(uint32_t);
        // offset indices by the vertex count of previous meshes
        size_t vertCount = _vertexBuffers[i]->length() / sizeof(Vertex);
        for (size_t j = 0; j < count; j++)
            all.push_back(ptr[j] + vertexBase);
        vertexBase += vertCount;
    }
    return device->newBuffer(all.data(), all.size() * sizeof(uint32_t), MTL::ResourceStorageModeShared);
}

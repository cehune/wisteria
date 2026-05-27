//
//  SceneGeometryPool.cpp
//  terrace
//
//  Created by celine on 2026-03-21.
//

#include "SceneGeometryPool.hpp"
#include <cassert>

SceneGeometryPool::SceneGeometryPool() {
    _meshLoader = MeshLoader();
}

SceneGeometryPool::~SceneGeometryPool() {
    if (_vertexBuffer) _vertexBuffer->release();
    if (_indexBuffer)  _indexBuffer->release();
}

// ── Internal: append geometry to staging vectors, rebasing indices ─────────
void SceneGeometryPool::_appendGeometry(Mesh& mesh,
                                        const std::vector<Vertex>& verts,
                                        const std::vector<uint32_t>& indices)
{
    // Record offsets BEFORE appending (these are the positions of the
    // new run's first element in element units, not bytes).
    const uint32_t vertexBase = static_cast<uint32_t>(_vertices.size());
    const uint32_t indexBase  = static_cast<uint32_t>(_indices.size());

    mesh.vertexOffset = vertexBase;
    mesh.indexOffset  = indexBase;
    mesh.vertexCount  = static_cast<uint32_t>(verts.size());
    mesh.indexCount   = static_cast<uint32_t>(indices.size());
    mesh.numTriangles = static_cast<uint32_t>(indices.size() / 3);

    // Append vertices verbatim.
    _vertices.insert(_vertices.end(), verts.begin(), verts.end());

    // Append indices, rebased to mega vertex buffer space (global indices).
    _indices.reserve(_indices.size() + indices.size());
    for (uint32_t i : indices) {
        _indices.push_back(i + vertexBase);
    }

    auto [bmin, bmax] = calculateBounds(verts);
    mesh.localBoundsMin = bmin;
    mesh.localBoundsMax = bmax;

    _dirty = true;
}

void SceneGeometryPool::uploadMesh(Mesh& mesh,
                                   const std::vector<Vertex>& verts,
                                   const std::vector<uint32_t>& indices,
                                   MTL::Device* device)
{
    if (!_device) _device = device;
    _appendGeometry(mesh, verts, indices);
}

void SceneGeometryPool::uploadMeshFile(Mesh& mesh, const std::string& meshPath, MTL::Device* device) {
    if (!_device) _device = device;

    std::vector<Vertex>   verts;
    std::vector<uint32_t> indices;
    _meshLoader.loadObjMesh(meshPath, verts, indices);

    _appendGeometry(mesh, verts, indices);
}

/* Calculates bounds of a vector of vertices. Vec4 because it returns the min and max POSITION. */
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

// ── GPU buffer (re)build ────────────────────────────────────────────────────
void SceneGeometryPool::_rebuildGPUBuffersIfDirty() {
    if (!_dirty) return;
    assert(_device && "SceneGeometryPool: no device cached; call uploadMesh* first");

    // Buffers can't be resized in Metal, so a new upload means new allocations.
    // (TODO: Future optimization: pre-reserve capacity and bump-append; double on overflow.)
    if (_vertexBuffer) { _vertexBuffer->release(); _vertexBuffer = nullptr; }
    if (_indexBuffer)  { _indexBuffer->release();  _indexBuffer  = nullptr; }

    if (!_vertices.empty()) {
        _vertexBuffer = _device->newBuffer(_vertices.data(),
                                           _vertices.size() * sizeof(Vertex),
                                           MTL::ResourceStorageModeShared);
    }
    if (!_indices.empty()) {
        _indexBuffer = _device->newBuffer(_indices.data(),
                                          _indices.size() * sizeof(uint32_t),
                                          MTL::ResourceStorageModeShared);
    }

    _dirty = false;
}

void SceneGeometryPool::finalize() {
    _rebuildGPUBuffersIfDirty();
}

MTL::Buffer* SceneGeometryPool::vertexBuffer() {
    _rebuildGPUBuffersIfDirty();
    return _vertexBuffer;
}

MTL::Buffer* SceneGeometryPool::indexBuffer() {
    _rebuildGPUBuffersIfDirty();
    return _indexBuffer;
}

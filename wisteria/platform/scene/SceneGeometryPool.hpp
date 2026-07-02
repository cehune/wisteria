//
//  SceneGeometryPool.hpp
//  wisteria
//
//  Created by celine on 2026-03-20.
#pragma once
#include <vector>
#include <Metal/Metal.hpp>
#include "engine/scene/Mesh.hpp"
#include "engine/scene/Core.hpp" // 6429
#include "IGeometryPool.hpp"

/*
 SceneGeometryPool consolidates all scene geometry into two megabuffers
 (one for vertices, one for indices). Meshes uploaded via uploadMesh have
 their geometry appended to CPU-side staging vectors; indices are rebased
 to the mega vertex buffer's space at append time.

 GPU buffers are (re)built lazily from the staging vectors on the next
 vertexBuffer()/indexBuffer() call after any upload. Call finalize() to
 force the build at a known-good point (e.g. after all scene loads, before
 the first draw).

 The MTL::Device used to allocate GPU buffers is cached from the first
 upload* call. Calling a getter before any upload is undefined.
 */
class SceneGeometryPool: public IGeometryPool {
public:
    SceneGeometryPool();
    ~SceneGeometryPool() override;

    void uploadMesh(Mesh& mesh,
                    const std::vector<Vertex>& verts,
                    const std::vector<uint32_t>& indices,
                    MTL::Device* device) override;

    std::pair<Vec4, Vec4> calculateBounds(const std::vector<Vertex>& verts);

    MTL::Buffer* vertexBuffer() override;
    MTL::Buffer* indexBuffer() override;
    void finalize() override;

    // Inspection (mainly for tests)
    size_t stagedVertexCount() const { return _vertices.size(); }
    size_t stagedIndexCount()  const { return _indices.size();  }

private:
    void _rebuildGPUBuffersIfDirty();
    void _appendGeometry(Mesh& mesh,
                         const std::vector<Vertex>& verts,
                         const std::vector<uint32_t>& indices);

    // CPU-side staging (source of truth).
    std::vector<Vertex>   _vertices;
    std::vector<uint32_t> _indices;

    // GPU mega buffers. Rebuilt from staging when _dirty.
    MTL::Buffer* _vertexBuffer = nullptr;
    MTL::Buffer* _indexBuffer  = nullptr;
    bool         _dirty        = false;

    MTL::Device* _device       = nullptr; // cached from first upload
};

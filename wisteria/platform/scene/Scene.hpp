//
//  Scene.hpp 
//  wisteria
//
//  Created by celine on 2026-03-20.
//

#pragma once
#include <Metal/Metal.hpp>
#include "engine/scene/Mesh.hpp"
#include "engine/scene/Core.hpp"
#include "SceneGeometryPool.hpp"
#include <simd/simd.h>
#include <unordered_map>

class Scene {
public:
    Scene(IGeometryPool& pool);
    void loadMesh(const std::string& path, uint32_t materialID,
                     const simd::float4x4& transform, MTL::Device* device);
    void addMeshDirect(Mesh& mesh,
                       const std::vector<Vertex>& verts,
                       const std::vector<uint32_t>& indices,
                       MTL::Device* device);
    
    // First call adds mesh and mesh instance. Subsequent calls with same filepath only do instance.
    // Returns false (and adds nothing) if the mesh fails to load.
    bool addMeshInstance(const std::string& meshfilepath,
                         const simd::float4x4& transform,
                         MTL::Device* device);

    // call before render for setup
    // void finalize(MTL::Device* device);

    IGeometryPool& geometryPool() { return _pool; }
    MTL::Buffer* materialBuffer()     { return _materialBuffer; }
    std::vector<Mesh>& meshes() { return _meshes; }
    std::vector<MeshInstance>& instances() { return _meshInstances; }
    
    // Getters mainly for testing
    const Mesh& mesh(size_t i) const { return _meshes[i]; }
    const MeshInstance& instance(size_t i) const { return _meshInstances[i]; }
    const size_t numMeshes() { return _meshes.size(); }
    const size_t numMeshInstances() { return _meshInstances.size(); }
    
private:
    std::vector<Mesh>               _meshes;
    std::vector<MeshInstance>       _meshInstances;
    std::unordered_map<std::string, int> _fileToMeshIndex;

    // materials
    IGeometryPool&                  _pool;
    MTL::Buffer*                    _materialBuffer = nil;
};

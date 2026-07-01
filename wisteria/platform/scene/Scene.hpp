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
#include "engine/scene/Material.hpp"
#include "SceneGeometryPool.hpp"
#include <simd/simd.h>
#include <unordered_map>

class Scene {
public:
    Scene(IGeometryPool& pool);
    ~Scene() { if (_materialBuffer) _materialBuffer->release(); }
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
                         MTL::Device* device,
                         uint32_t materialID = 0);

    // Materials (descriptions). materialBuffer() is (re)built by buildMaterialBuffer().
    uint32_t addMaterial(const Material& m) { _materials.push_back(m); return (uint32_t)_materials.size() - 1; }
    std::vector<Material>& materials() { return _materials; }
    void buildMaterialBuffer(MTL::Device* device);

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
    std::vector<Material>           _materials;
    std::unordered_map<std::string, int> _fileToMeshIndex;

    // materials
    IGeometryPool&                  _pool;
    MTL::Buffer*                    _materialBuffer = nil;
};

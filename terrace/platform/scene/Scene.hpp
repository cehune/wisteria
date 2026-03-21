//
//  Scene.hpp 
//  terrace
//
//  Created by celine on 2026-03-20.
//

#pragma once
#include <Metal/Metal.hpp>
#include "engine/scene/Mesh.hpp"
#include "engine/scene/Core.hpp"
#include "MeshLoader.hpp"
#include "SceneGeometryPool.hpp"
#include <simd/simd.h>

class Scene {
public:
    
    void loadMesh(const std::string& path, uint32_t materialID,
                     const simd::float4x4& transform, MTL::Device* device);
    void addMeshDirect(Mesh& mesh,
                       const std::vector<Vertex>& verts,
                       const std::vector<uint32_t>& indices,
                       MTL::Device* device);
    
    void addMeshInstance(Mesh& mesh, const Mat4& transform);

    // call before render for setup
    // void finalize(MTL::Device* device);

    SceneGeometryPool& geometryPool() { return _pool; }
    MTL::Buffer* materialBuffer()     { return _materialBuffer; }
    
private:
    std::vector<Mesh>               _meshes;
    std::vector<MeshInstance>       _meshInstances;
    // materials
    SceneGeometryPool               _pool;
    MTL::Buffer*                    _materialBuffer = nil;
    MeshLoader                      _loader;
};

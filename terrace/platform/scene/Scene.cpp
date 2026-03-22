//
//  Scene.cpp
//  terrace
//
//  Created by celine on 2026-03-21.
//

#include "Scene.hpp"


void Scene::addMeshDirect(Mesh& mesh,
                           const std::vector<Vertex>& verts,
                           const std::vector<uint32_t>& indices,
                           MTL::Device* device)
{
    _pool.uploadMesh(mesh, verts, indices, device);
    mesh.index = _meshes.size();
    _meshes.push_back(mesh);
    
    return;
}

void Scene::addMeshInstance(Mesh& mesh, const Mat4& transform)
{
    MeshInstance instance;
    
    instance.meshIndex = mesh.index;
    
    instance.transform = transform;
    
    instance.index = _meshInstances.size();
    instance.boundsMin = simd_mul(transform, mesh.localBoundsMin);
    instance.boundsMax = simd_mul(transform, mesh.localBoundsMax);
    return;
}

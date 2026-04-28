//
//  Scene.cpp
//  terrace
//
//  Created by celine on 2026-03-21.
//

#include "Scene.hpp"

Scene::Scene(IGeometryPool& pool): _pool(pool) {}

void Scene::addMeshDirect(Mesh& mesh,
                           const std::vector<Vertex>& verts,
                           const std::vector<uint32_t>& indices,
                           MTL::Device* device)
{
    _pool.uploadMesh(mesh, verts, indices, device);
    mesh.index = _meshes.size();
    _meshes.push_back(mesh);
}

void Scene::addMeshInstance(std::string& mesh_file_path, MTL::Device* device)
{
    uint16_t meshIndex;
    auto iter = _fileToMeshIndex.find(mesh_file_path);
    
    // generate new mesh and new instance
    if (iter == _fileToMeshIndex.end()) {
        Mesh mesh;
        _pool.uploadMeshFile(mesh, mesh_file_path, device);
        meshIndex = _meshes.size();
        mesh.index = meshIndex;
        _meshes.push_back(mesh);
        _fileToMeshIndex[mesh_file_path] = meshIndex;
        
    } else { // otherwise the mesh has already been uploaded, don't need to re-upload!
        meshIndex = iter->second;
    }
    
    // then we create a mesh index
    Mesh& mesh = _meshes[meshIndex];
    MeshInstance instance;
    instance.index = _meshInstances.size();
    instance.meshIndex = mesh.index;
    instance.boundsMin = mesh.localBoundsMin;
    instance.boundsMax = mesh.localBoundsMax;
    // identity transform for now
    instance.transform = matrix_identity_float4x4;
    
    mesh.meshInstanceIndexes.push_back(instance.index);
    _meshInstances.push_back(instance);
}

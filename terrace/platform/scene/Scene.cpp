//
//  Scene.cpp
//  terrace
//
//  Created by celine on 2026-03-21.
//

#include "Scene.hpp"
#include <iostream>

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
    Mesh mesh;
    // TODO: add check for mesh hashing, implement mesh instance vs mesh object
    _pool.uploadMeshFile(mesh, mesh_file_path, device);
    std::cout << "woawh";
    mesh.index = _meshes.size();
    _meshes.push_back(mesh);
}

//
//  Scene.cpp
//  wisteria
//
//  Created by celine on 2026-03-21.
//

#include "Scene.hpp"
#include <exception>
#include <iostream>

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

bool Scene::addMeshInstance(const std::string& mesh_file_path,
                            const simd::float4x4& transform,
                            MTL::Device* device)
{
    uint32_t meshIndex;
    auto iter = _fileToMeshIndex.find(mesh_file_path);

    // generate new mesh and new instance
    if (iter == _fileToMeshIndex.end()) {
        Mesh mesh;
        // The loader throws on a bad path / parse error. Skip instances that fail.
        try {
            _pool.uploadMeshFile(mesh, mesh_file_path, device);
        } catch (const std::exception& e) {
            std::cerr << "[Scene] failed to load mesh '" << mesh_file_path
                      << "': " << e.what() << " - skipping instance\n";
            return false;
        }
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
    instance.materialID = 0;   // TODO: real material assignment
    instance.pipelineID = 0;   // TODO: per-instance pipeline selection
    instance.boundsMin = mesh.localBoundsMin;
    instance.boundsMax = mesh.localBoundsMax;
    instance.transform = transform;

    // mirror megabuffer offsets onto the instance so it's self-sufficient
    instance.vertexOffset = mesh.vertexOffset;
    instance.indexOffset  = mesh.indexOffset;
    instance.indexCount   = mesh.indexCount;

    mesh.meshInstanceIndexes.push_back(instance.index);
    _meshInstances.push_back(instance);
    return true;
}

//
//  Scene.cpp
//  wisteria
//
//  Created by celine on 2026-03-21.
//

#include "Scene.hpp"
#include "engine/geometry/MeshLoader.hpp"   // ObjSubmesh + loadObjSubmeshes
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
                            MTL::Device* device,
                            uint32_t materialID)
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
    instance.materialID = materialID;
    instance.lightID = -1;
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

void Scene::buildMaterialBuffer(MTL::Device* device) {
    if (_materials.empty()) return;
    if (_materialBuffer) _materialBuffer->release();
    _materialBuffer = device->newBuffer(_materials.data(),
                                        _materials.size() * sizeof(Material),
                                        MTL::ResourceStorageModeShared);
}

uint32_t Scene::addLight(const Light& L, uint32_t instanceIndex) {
    uint32_t id = (uint32_t)_lights.size();
    _lights.push_back(L);
    _lights.back().instanceID = instanceIndex;
    _meshInstances[instanceIndex].lightID = (int32_t)id;
    return id;
}

void Scene::buildLightBuffer(MTL::Device* device) {
    if (_lights.empty()) return;
    if (_lightBuffer) _lightBuffer->release();
    _lightBuffer = device->newBuffer(_lights.data(),
                                     _lights.size()*sizeof(Light),
                                     MTL::ResourceStorageModeShared);
}

uint32_t Scene::addMeshFromData(const std::vector<Vertex>& verts,
                                const std::vector<uint32_t>& indices,
                                const simd::float4x4& transform,
                                uint32_t materialID, MTL::Device* device) {
    Mesh mesh;
    mesh.name = "submesh";
    addMeshDirect(mesh, verts, indices, device);  // uploads + sets offsets/counts/bounds + mesh.index

    MeshInstance instance;
    instance.index        = (uint32_t)_meshInstances.size();
    instance.meshIndex    = (uint32_t)mesh.index;
    instance.materialID   = materialID;
    instance.lightID      = -1;
    instance.pipelineID   = 0;
    instance.boundsMin    = mesh.localBoundsMin;
    instance.boundsMax    = mesh.localBoundsMax;
    instance.transform    = transform;
    instance.vertexOffset = mesh.vertexOffset;
    instance.indexOffset  = mesh.indexOffset;
    instance.indexCount   = mesh.indexCount;

    _meshes[mesh.index].meshInstanceIndexes.push_back(instance.index);
    _meshInstances.push_back(instance);
    return instance.index;
}

void Scene::loadObjScene(const std::string& path,
                         const simd::float4x4& transform, MTL::Device* device) {
    std::vector<ObjSubmesh> submeshes;
    MeshLoader().loadObjSubmeshes(path, submeshes);

    for (const ObjSubmesh& s : submeshes) {
        uint32_t materialID = addMaterial({ MATERIAL_LAMBERTIAN, s.albedo });
        uint32_t instanceID = addMeshFromData(s.vertices, s.indices, transform, materialID, device);

        // Ke > 0  ->  this submesh is an emitter; addLight() sets the two-way link.
        if (simd_length(s.emission) > 0.0f) {
            Light light{};
            light.type     = LIGHT_AREA;
            light.radiance = s.emission;
            light.twoSided = 0;   // one-sided (Cornell ceiling faces down)
            addLight(light, instanceID);
        }
    }
}

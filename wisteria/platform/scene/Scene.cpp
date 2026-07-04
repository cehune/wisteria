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

uint32_t Scene::addMesh(const std::vector<Vertex>& verts,
                        const std::vector<uint32_t>& indices,
                        MTL::Device* device) {
    Mesh mesh;
    mesh.name = "mesh";
    _pool.uploadMesh(mesh, verts, indices, device);   // sets offsets / counts / bounds
    mesh.index = (uint32_t)_meshes.size();
    _meshes.push_back(mesh);
    return (uint32_t)mesh.index;
}

uint32_t Scene::addInstance(uint32_t meshIndex,
                            const simd::float4x4& transform,
                            uint32_t materialID) {
    const Mesh& mesh = _meshes[meshIndex];   // reused across instances -> no re-upload

    MeshInstance instance;
    instance.index        = (uint32_t)_meshInstances.size();
    instance.meshIndex    = meshIndex;
    instance.materialID   = materialID;
    instance.lightID      = -1;
    instance.pipelineID   = 0;
    instance.boundsMin    = mesh.localBoundsMin;
    instance.boundsMax    = mesh.localBoundsMax;
    instance.transform    = transform;
    instance.vertexOffset = mesh.vertexOffset;
    instance.indexOffset  = mesh.indexOffset;
    instance.indexCount   = mesh.indexCount;

    _meshes[meshIndex].meshInstanceIndexes.push_back(instance.index);
    _meshInstances.push_back(instance);
    return instance.index;
}

uint32_t Scene::addMeshFromData(const std::vector<Vertex>& verts,
                                const std::vector<uint32_t>& indices,
                                const simd::float4x4& transform,
                                uint32_t materialID, MTL::Device* device) {
    return addInstance(addMesh(verts, indices, device), transform, materialID);
}

void Scene::loadObjScene(const std::string& path,
                         const simd::float4x4& transform, MTL::Device* device) {
    std::vector<ObjSubmesh> submeshes;
    // loads the data into the submeshes
    Geometry::loadObjSubmeshes(path, submeshes);

    for (const ObjSubmesh& s : submeshes) {
        // create mat
        Material gpuMaterial;
        if (s.metallic > 0.5f) {                 // metallic -> conductor (Kd reused as F0, Pr as roughness)
            gpuMaterial.type      = MATERIAL_CONDUCTOR;
            gpuMaterial.albedo    = s.albedo;
            gpuMaterial.roughness = s.roughness;
        } else {
            gpuMaterial.type      = MATERIAL_LAMBERTIAN;
            gpuMaterial.albedo    = s.albedo;
            gpuMaterial.roughness = 0.0f;
        }
        uint32_t materialID = addMaterial(gpuMaterial);
        // upload the geometry and create the instance
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

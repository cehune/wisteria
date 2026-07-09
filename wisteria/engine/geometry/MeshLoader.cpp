//
//  MeshLoader.cpp
//  wisteria
//
//  Created by celine on 2026-04-21.
//

#include "MeshLoader.hpp"
#include <iostream>

void Geometry::Internal::getMeshNormals(std::vector<Vertex>& vertices,
                                std::vector<uint32_t>& indices) {
    for (uint32_t i = 0; i < indices.size(); i += 3) {
        Vertex& vA = vertices[indices[i]];
        Vertex& vB = vertices[indices[i + 1]];
        Vertex& vC = vertices[indices[i + 2]];

        Vec3 a = {vA.position.x, vA.position.y, vA.position.z};
        Vec3 b = {vB.position.x, vB.position.y, vB.position.z};
        Vec3 c = {vC.position.x, vC.position.y, vC.position.z};

        Vec3 n = simd_normalize(simd_cross(b - a, c - a));
        vA.normal = {n.x, n.y, n.z, 0.0};
        vB.normal = {n.x, n.y, n.z, 0.0};
        vC.normal = {n.x, n.y, n.z, 0.0};
    }
}

void Geometry::loadObjSubmeshes(const std::string& meshPath, std::vector<ObjSubmesh>& out) {
    if (meshPath.empty()) throw std::runtime_error("bad mesh path");

    rapidobj::Result result = rapidobj::ParseFile(meshPath);
    if (result.error) throw std::runtime_error(result.error.code.message());
    rapidobj::Triangulate(result);

    std::unordered_map<int, size_t> matToSubIndex;

    for (const auto& shape : result.shapes) {
        Internal::processShape(shape, result, out, matToSubIndex);
    }

    // Post-processing
    if (result.attributes.normals.empty()) {
        for (auto& submesh : out) {
            Internal::getMeshNormals(submesh.vertices, submesh.indices);
        }
    }
}


void Geometry::Internal::processShape(const rapidobj::Shape& shape, const rapidobj::Result& result,
                             std::vector<ObjSubmesh>& out, std::unordered_map<int, size_t>& matToSub) {
    
    for (size_t tri = 0; tri < shape.mesh.indices.size() / 3; ++tri) { // For each triangle
        int matID = (tri < shape.mesh.material_ids.size()) ? shape.mesh.material_ids[tri] : -1;
        // Set the object submesh
        ObjSubmesh& submesh = getOrCreateSubmesh(matID, result, out, matToSub);

        for (int k = 0; k < 3; ++k) {
            // apply vertices on top of each
            submesh.vertices.push_back(extractVertex(shape.mesh.indices[3 * tri + k], result));
            submesh.indices.push_back((uint32_t)submesh.vertices.size() - 1);
        }
    }
}

Vertex Geometry::Internal::extractVertex(const rapidobj::Index& ix, const rapidobj::Result& res) {
    // reads a given POINT VECTOR in the mesh, and then for a given submesh, each vertex should
    // correspond uniquely to one
    Vertex v{};
    
    // position (always present for a valid OBJ, but guard so a malformed file
    // fails loudly upstream instead of indexing out of bounds here)
    if (ix.position_index >= 0 && !res.attributes.positions.empty()) {
        const float* pos = &res.attributes.positions[ix.position_index * 3];
        v.position = { pos[0], pos[1], pos[2], 1.0f };
    }

    // normals
    if (ix.normal_index >= 0 && !res.attributes.normals.empty()) {
        const float* norm = &res.attributes.normals[ix.normal_index * 3];
        v.normal = { norm[0], norm[1], norm[2], 0.0f }; // W=0 for vectors
    } else {
        v.normal = { 0.0f, 0.0f, 0.0f, 0.0f };
    }

    // texcoords
    if (ix.texcoord_index >= 0 && !res.attributes.texcoords.empty()) {
        const float* tex = &res.attributes.texcoords[ix.texcoord_index * 2];
        v.uv = { tex[0], tex[1] };
    }
    
    v.tangent = {1, 0, 0, 0};
    v.color = {1, 1, 1, 1};
    return v;
}

ObjSubmesh& Geometry::Internal::getOrCreateSubmesh(int matID,
                                          const rapidobj::Result& result,
                                          std::vector<ObjSubmesh>& out,
                                          std::unordered_map<int, size_t>& matToSub) {
    // check if we already have a submesh for this materialmatToSub
    auto it = matToSub.find(matID);
    if (it != matToSub.end()) {
        return out[it->second]; // submesh itself
    }

    // create a new submesh
    out.emplace_back();
    ObjSubmesh& submesh = out.back();
    
    // assign material properties if available
    if (matID >= 0 && matID < (int)result.materials.size()) {
        const auto& m = result.materials[matID];
        submesh.materialName = m.name;
        submesh.albedo       = { m.diffuse[0], m.diffuse[1], m.diffuse[2] };
        submesh.emission     = { m.emission[0], m.emission[1], m.emission[2] };
        submesh.roughness    = m.roughness;
        submesh.metallic     = m.metallic;
        submesh.ior          = m.ior;
        submesh.dissolve     = m.dissolve;
    } else {
        // fallback for default material
        submesh.materialName = "default";
        submesh.albedo       = { 0.5f, 0.5f, 0.5f };
    }

    // register in map
    matToSub[matID] = out.size() - 1;
    return submesh;
}

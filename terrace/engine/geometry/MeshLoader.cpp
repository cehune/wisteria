//
//  MeshLoader.cpp
//  terrace
//
//  Created by celine on 2026-04-21.
//

#include "MeshLoader.hpp"
#include <iostream>

void MeshLoader::loadObjMesh(const std::string& meshPath,
                std::vector<Vertex>& vertices,
                std::vector<uint32_t>& indices) {
    if (meshPath.empty()) throw std::runtime_error("Attempted to load bad mesh path");

    rapidobj::Result result = rapidobj::ParseFile(meshPath);
    
    if (result.error) throw std::runtime_error(result.error.code.message());
    rapidobj::Triangulate(result); // just in case its quads lol
    bool normal_flag = true; // assume normals are included in the obj
    
    for (const rapidobj::Shape& shape: result.shapes) {
        for (const auto& index: shape.mesh.indices) {
            Vertex v{};
            
            // positions
            int pos_index = index.position_index;
            if (pos_index >= 0 && !result.attributes.positions.empty()) {
                v.position = {
                    result.attributes.positions[pos_index * 3],
                    result.attributes.positions[pos_index * 3 + 1],
                    result.attributes.positions[pos_index * 3 + 2],
                    1.0
                };
            }
            
            // normals
            int norm_index = index.normal_index;
            if (norm_index >= 0 && !result.attributes.normals.empty()) {
                v.normal = {
                    result.attributes.normals[norm_index * 3],
                    result.attributes.normals[norm_index * 3 + 1],
                    result.attributes.normals[norm_index * 3 + 2]
                };
            } else {
                normal_flag = false;
            }
            
            // texture uvs
            int tex_index = index.texcoord_index;
            if (tex_index >= 0 && !result.attributes.texcoords.empty()) {
                v.uv = {
                    result.attributes.texcoords[tex_index * 2],
                    result.attributes.texcoords[tex_index * 2 + 1]
                };
            }
            indices.push_back(static_cast<uint32_t>(vertices.size()));
            vertices.push_back(v);
        }
    }
    if (!normal_flag) {
        getMeshNormals(vertices, indices);
    }
    printVertex(vertices.front());
        
}


void MeshLoader::getMeshNormals(std::vector<Vertex>& vertices,
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

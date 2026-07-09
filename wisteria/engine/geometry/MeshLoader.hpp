//
//  MeshLoader.hpp
//  wisteria
//
//  Created by celine on 2026-04-21.
//
#pragma once

#include "rapidobj.hpp"
#include "engine/scene/Core.hpp"
#include <unordered_map>
#include <string>

#include <simd/simd.h>
struct ObjSubmesh {
    std::string           materialName;
    simd_float3           albedo   {0.5f,0.5f,0.5f};   // Kd
    simd_float3           emission {0.0f,0.0f,0.0f};   // Ke  -> nonzero = area light
    float roughness = 0.0f, metallic = 0.0f, ior = 1.0f;  // Pr/Pm/Ni, for later BxDFs
    float dissolve = 1.0f;  // for dielectric
    std::vector<Vertex>   vertices;
    std::vector<uint32_t> indices;
};

namespace Geometry {
    // High-level: The only function the Scene needs to call
    void loadObjSubmeshes(const std::string& meshPath, std::vector<ObjSubmesh>& out);

    // Internal utilities (don't expose these to the Scene)
    namespace Internal {
        void processShape(const rapidobj::Shape& shape, const rapidobj::Result& result,
                             std::vector<ObjSubmesh>& out, std::unordered_map<int, size_t>& matToSub);
        void getMeshNormals(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
        ObjSubmesh& getOrCreateSubmesh(int matID, const rapidobj::Result& result,
                                       std::vector<ObjSubmesh>& out,
                                       std::unordered_map<int, size_t>& matToSub);
        Vertex extractVertex(const rapidobj::Index& ix, const rapidobj::Result& res);
    }
}
//
//  MeshLoader.hpp
//  terrace
//
//  Created by celine on 2026-04-21.
//
#pragma once

#include "rapidobj.hpp"
#include "engine/scene/Core.hpp"
#include <string>

class MeshLoader {
public:
    void loadObjMesh(const std::string& meshPath,
                                std::vector<Vertex>& vertices,
                                std::vector<uint32_t>& indices);
    //TODO: GLTF LOADING!!!!
    
    void getMeshNormals(std::vector<Vertex>& vertices,
                        std::vector<uint32_t>& indices);
};

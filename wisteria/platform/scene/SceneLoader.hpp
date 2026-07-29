//
//  SceneLoader.hpp
//  wisteria
//
//  Created by celine on 2026-07-28.
//  One scene-construction path. This is for loading a single mesh as the default. 
//  ie. terminology for this is 'obj scene'. 
//  A 'Scene' in this project is the entire world of meshes, this is just for one mesh, esp
//  because the headless path obviously doesn't have any editing capabilities
//  TODO: make it so it also moves camera pos and the window proportions to match that of
//  the scene
//
#pragma once
#include <Metal/Metal.hpp>
#include <simd/simd.h>
#include <memory>
#include <string>

#include "platform/scene/Scene.hpp"
#include "platform/scene/SceneGeometryPool.hpp"
#include "engine/io/AssetPath.hpp"

// Cornell box as default, just sets up geometry and returns a scene
inline std::unique_ptr<Scene> loadScene(MTL::Device* device,
                                        const std::string& objPath = "") {
    auto scene = std::make_unique<Scene>(std::make_unique<SceneGeometryPool>());

    const std::string path = objPath.empty()
        ? wisteria::assets::samplePath("cornell_box.obj")
        : objPath;

    scene->loadObjScene(path, matrix_identity_float4x4, device);

    // Fill the GPU mega buffers once everything is staged.
    scene->geometryPool().finalize();
    return scene;
}

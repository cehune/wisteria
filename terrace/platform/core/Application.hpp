//
//  Application.hpp
//  terrace
//
//  Created by celine on 2026-03-12.
//

#pragma once
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include "platform/renderer/backend/RenderBackend.hpp"
#include "platform/renderer/backend/PathTracerBackend.hpp"
#include <iostream>
#include "Renderer.hpp"
#include "Scene.hpp"

class Application {
public:
    // Override if we do a non-metal version of this codebase
    Application(MTL::Device* _device);
    void update();
    void render(const FrameContext& ctx);
    void onResize(uint32_t width, uint32_t height);
    void shutdown();
    void run();
        
private:
    void init(MTL::Device* device);

    // GPU entry point
    MTL::Device* device = nullptr;
    std::unique_ptr<Scene>    scene;
    std::unique_ptr<IGeometryPool> pool;
    std::unique_ptr<Renderer> renderer;
};

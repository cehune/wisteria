//
//  Application.hpp
//  wisteria
//
//  Created by celine on 2026-03-12.
//

#pragma once
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include "platform/renderer/backend/IRenderBackend.hpp"
#include "platform/renderer/backend/PathTracerBackend.hpp"
#include <iostream>
#include "Renderer.hpp"
#include "Scene.hpp"

enum class BackendType { Raster, PathTracer };

class Application {
public:
    // Override if we do a non-metal version of this codebase
    Application(MTL::Device* _device);
    void update();
    void render(const FrameContext& ctx);
    void onResize(uint32_t width, uint32_t height);
    void shutdown();
    void run();
    
    // called via metalview
    void onKey(int key, bool pressed);
    void onScroll(float delta);
    void onMouseDrag(float dx, float dy);
        
private:
    void init(MTL::Device* device);
    std::unique_ptr<IRenderBackend> makeBackend(BackendType type);

    // Flip to BackendType::PathTracer to run the compute path tracer.
    BackendType _backend = BackendType::Raster;

    // GPU entry point
    MTL::Device* device = nullptr;
    std::unique_ptr<Scene>    scene;
    std::unique_ptr<IGeometryPool> pool;
    std::unique_ptr<Renderer> renderer;
};

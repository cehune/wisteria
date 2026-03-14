//
//  Application.hpp
//  terrace
//
//  Created by celine on 2026-03-12.
//

#pragma once
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <iostream>
#include "Renderer.hpp"

class Application {
public:
    // Override if we do a non-metal version of this codebase
    Application(MTL::Device* _device);
    void update();
    void render(MTL::RenderPassDescriptor* desc, MTL::Drawable* drawable);
    void shutdown();

    void run();
        
private:
    void init(MTL::Device* device);

    // GPU entry point
    MTL::Device* device = nullptr;
    MTL::CommandQueue* commandQueue = nullptr;

    // Renderer (your engine)
    std::unique_ptr<Renderer> renderer;
};

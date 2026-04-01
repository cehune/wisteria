//
//  PathTracerBackend.hpp
//  terrace
//
//  Created by celine on 2026-03-31.
//

#pragma once
#include "RenderBackend.hpp"
#include <Metal/Metal.hpp>
#include "platform/scene/Scene.hpp"
#include <QuartzCore/QuartzCore.hpp>
#include <iostream>

class PathTracerBackend : public RenderBackend {
public:
    PathTracerBackend(MTL::Device* device, Scene* scene);
    ~PathTracerBackend();
    
    void draw(const FrameContext& ctx) override;
    void onResize(uint32_t width, uint32_t height) override;
    
private:
    void _buildPipeline();
    void _buildOffscreenTexture(uint32_t w, uint32_t h);
    MTL::Device*                     _device;
    Scene*                           _scene;
    MTL::CommandQueue*               _commandQueue;
    
    MTL::ComputePipelineState* _pso       = nullptr;
    MTL::Texture*              _offscreen = nullptr;
    uint32_t _width = 0;
    uint32_t _height = 0;
};

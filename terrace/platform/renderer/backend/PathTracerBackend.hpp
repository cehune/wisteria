//
//  PathTracerBackend.hpp
//  terrace
//
//  Created by celine on 2026-03-31.
//

#pragma once
#include "IRenderBackend.hpp"
#include <Metal/Metal.hpp>
#include "platform/scene/Scene.hpp"
#include <QuartzCore/QuartzCore.hpp>
#include <iostream>
#include "engine/scene/CameraUniforms.hpp"

class PathTracerBackend : public IRenderBackend {
public:
    PathTracerBackend(MTL::Device* device, Scene* scene);
    ~PathTracerBackend();
    
    void draw(const FrameContext& ctx) override;
    void onResize(uint32_t width, uint32_t height) override;
    
    // Don't really do these for path tracing for now.
    void onKey(int key, bool pressed) override { return; };
    void onScroll(float delta) override { return; };
    void onMouseDrag(float dx, float dy) override { return; };
    
private:
    void _buildPipeline();
    void _buildOffscreenTexture(uint32_t w, uint32_t h);
    void _updateCameraBuffer();

    MTL::Device*                    _device;
    Scene*                          _scene;
    MTL::CommandQueue*              _commandQueue;
    MTL::ComputePipelineState*      _pso       = nullptr;
    MTL::Texture*                   _offscreen = nullptr;
    MTL::Buffer*                    _cameraBuffer = nullptr;
    CameraUniformsPT                  _camera;
    uint32_t _width = 800;
    uint32_t _height = 600;
};

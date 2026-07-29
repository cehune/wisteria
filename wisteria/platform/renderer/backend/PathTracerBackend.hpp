//
//  PathTracerBackend.hpp
//  wisteria
//
//  Created by celine on 2026-03-31.
//

#pragma once
#include "IRenderBackend.hpp"
#include "AccelStructures.hpp"
#include <Metal/Metal.hpp>
#include "platform/scene/Scene.hpp"
#include <QuartzCore/QuartzCore.hpp>
#include <iostream>
#include "engine/scene/Camera/Camera.hpp"
#include "engine/io/PFM.h"
#include <filesystem>
#include <semaphore>

class PathTracerBackend : public IRenderBackend {
public:
    PathTracerBackend(MTL::Device* device, Scene* scene);
    ~PathTracerBackend();
    
    void draw(const FrameContext& ctx) override;
    void onResize(uint32_t width, uint32_t height) override;
    
    void onKey(int key, bool pressed) override;
    void onScroll(float delta) override { return; };
    void onMouseDrag(float dx, float dy) override { return; };
    void setCameraState(const CameraState& state);
    void exportCurrentImage(const std::string& path) override;
    
    // Production and display for the draw call
    void renderSamples(uint32_t n = 1);
    void presentRender(const FrameContext& ctx);
    void continueTo(uint32_t newTarget);
    
private:
    void _buildPipeline();
    void _buildOffscreenTexture(uint32_t w, uint32_t h);
    void _updateCameraBuffer();
    void _buildAccumulationTexture(uint32_t w, uint32_t h);

    bool _readbackAccumulationTexture(std::vector<float>& out,
                                      uint32_t& outW, uint32_t& outH);

    MTL::Device*                    _device;
    Scene*                          _scene;
    MTL::CommandQueue*              _commandQueue;
    MTL::ComputePipelineState*      _pso       = nullptr;
    MTL::Texture*                   _offscreen = nullptr;
    MTL::Buffer*                    _cameraBuffer = nullptr;
    CameraUniformsPT                _cameraUniforms;
    Camera                          _camera;
    CameraState                     _currentCameraState;

    // accumulation buffer
    MTL::Texture*                   _accumulation = nullptr;
    MTL::Texture*                   _moment2      = nullptr;
    bool                            _dirty        = true;
    uint32_t                        _sampleCount  = 0;
    uint32_t                        _maxSamples   = 20000;
    // triple buffer for draw commands if we want to add more than one sample per draw loop
    static constexpr uint32_t       _maxBuffers = 3;
    dispatch_semaphore_t            _drawBufferSemaphore;

    // acceleration structures (BLAS per mesh + TLAS over instances)
    AccelStructures                 _accel;
    bool                            _accelBuilt = false;

    uint32_t _width = 800;
    uint32_t _height = 600;
};

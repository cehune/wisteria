//
//  RasterBackend.hpp
//  terrace
//
//  Created by celine on 2026-03-30.
//
#pragma once
#include "RenderBackend.hpp"
#include <Metal/Metal.hpp>
#include "platform/renderer/PipelineLibrary.hpp"
#include "platform/scene/Scene.hpp"
#include "engine/scene/Camera.hpp"
#include "engine/scene/CameraUniforms.hpp"
#include <iostream>
#include <semaphore>

class RasterBackend : public RenderBackend {
public:
    RasterBackend(MTL::Device* device, Scene* scene);
    ~RasterBackend();
    void draw(const FrameContext& ctx) override;
    void onResize(uint32_t width, uint32_t height) override;

private:
    void          _updateCameraBuffer();
    
    MTL::Device*                     _device = nullptr;
    MTL::CommandQueue*               _commandQueue = nullptr;
    Scene*                           _scene;
    std::unique_ptr<PipelineLibrary> _pipelineLibrary;
    std::vector<PipelineKey>         _pipelines;
    Camera                           _camera;
    
    uint32_t _width  = 0;
    uint32_t _height = 0;
    
    float _aspect = 1.0f;
    float _fov    = 60.0f * M_PI / 180;
    float _near   = 0.1f;
    float _far    = 100.0f;
    
    /*
     TRIPLE BUFFER
     Since on the mac, the CPU and GPU work at the same time, we want to avoid writing and reading
     at the same time. Use 3 buffers frame by frame, where GPU will use the last, and then CPU will edit
     the next. use 3 buffer instead of 2 to prevent any waiting.
     */
    static constexpr uint8_t    _maxBuffers = 3;
    MTL::Buffer*                _cameraBuffers[_maxBuffers];
    uint8_t                     _frameIndex = 0;
    dispatch_semaphore_t        _camBufferSemaphore;
};

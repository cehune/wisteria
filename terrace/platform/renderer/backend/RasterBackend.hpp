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
#include <iostream>


class RasterBackend : public RenderBackend {
public:
    RasterBackend(MTL::Device* device, Scene* scene);
    ~RasterBackend() = default;
    void draw(MTL::RenderPassDescriptor* desc, MTL::Drawable* drawable) override;
    void onResize(uint32_t width, uint32_t height) override;

private:
    MTL::Device*                     _device;
    MTL::CommandQueue*               _commandQueue;
    Scene*                           _scene;
    std::unique_ptr<PipelineLibrary> _pipelineLibrary;
    std::vector<PipelineKey>         _pipelines;
};

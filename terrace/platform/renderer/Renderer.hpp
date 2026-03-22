//
//  Renderer.hpp
//  terrace
//
//  Created by celine on 2026-03-14.
//


#pragma once
#include <Metal/Metal.hpp>
#include "Scene.hpp"
#include "PipelineLibrary.hpp"
#include <QuartzCore/QuartzCore.hpp>
#include <iostream>


class Renderer {
public:
    Renderer(MTL::Device* device, Scene* scene);
    void draw(MTL::RenderPassDescriptor* desc, MTL::Drawable* drawable) ;
                         
    
private:
    MTL::Device* _device; // Application will own the device, renderer will just have reference
    Scene*                           _scene; // doesn't make sense for us to own here because we are just rendering
    MTL::CommandQueue*               _commandQueue;
    std::unique_ptr<PipelineLibrary> _pipelineLibrary;
    std::vector<PipelineKey>         _pipelines;
};

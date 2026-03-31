//
//  RenderBackend.hpp
//  terrace
//
//  Created by celine on 2026-03-30.
//
#pragma once
#include <Metal/Metal.hpp>
#include "Scene.hpp"

class RenderBackend {
public:
    virtual ~RenderBackend() = default;
    virtual void draw(MTL::RenderPassDescriptor* desc, MTL::Drawable* drawable) = 0;
    virtual void onResize(uint32_t width, uint32_t height) = 0;
};

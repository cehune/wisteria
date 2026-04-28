//
//  IRenderBackend.hpp
//  terrace
//
//  Created by celine on 2026-03-30.
//
#pragma once
#include <Metal/Metal.hpp>
#include "Scene.hpp"

struct FrameContext {
    MTL::RenderPassDescriptor* renderPassDesc;  // null for compute-only backends
    MTL::Drawable*             drawable;
    uint32_t                   width;
    uint32_t                   height;
    float                      dt;
};

class IRenderBackend {
public:
    virtual ~IRenderBackend() = default;
    virtual void draw(const FrameContext& ctx) = 0;
    virtual void onResize(uint32_t width, uint32_t height) = 0;
    
    // Input Events, expect to just forward to a camera controller
    virtual void onKey(int key, bool pressed) = 0;
    virtual void onScroll(float delta) = 0;
    virtual void onMouseDrag(float dx, float dy) = 0;
};


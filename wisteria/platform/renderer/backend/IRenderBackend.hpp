//
//  IRenderBackend.hpp
//  wisteria
//
//  Created by celine on 2026-03-30.
//
#pragma once
#include <Metal/Metal.hpp>
#include "Scene.hpp"
#include <string>

struct FrameContext {
    MTL::RenderPassDescriptor* renderPassDesc;  // null for compute-only backends
    MTL::Drawable*             drawable;
    uint32_t                   width;
    uint32_t                   height;
    float                      dt;
};

// Progressive integrators, only if they accumulate and converge over many iterations
class IProgressiveRenderer {
public:
    virtual ~IProgressiveRenderer() = default;

    // Accumulate n samples. Touches no drawable, so this is what an offline
    // driver uses to produce an image with no window and no vsync.
    virtual void renderSamples(uint32_t n) = 0;
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

    virtual void exportCurrentImage(const std::string& path) = 0;
    // Non-null only for backends that accumulate progressively. Callers that
    // need sample-level control query this instead of assuming a backend type.
    virtual IProgressiveRenderer* asProgressive() { return nullptr; }
};


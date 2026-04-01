//
//  Renderer.hpp
//  terrace
//
//  Created by celine on 2026-03-14.
//


#pragma once
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include "backend/RenderBackend.hpp"

class Renderer {
public:
    Renderer(std::unique_ptr<RenderBackend> backend);
    void draw(const FrameContext& ctx);
    void onResize(uint32_t width, uint32_t height);
private:
    std::unique_ptr<RenderBackend> _backend;
};

inline Renderer::Renderer(std::unique_ptr<RenderBackend> backend)
    : _backend(std::move(backend)) {}

inline void Renderer::draw(const FrameContext& ctx) {
    _backend->draw(ctx);
}

inline void Renderer::onResize(uint32_t width, uint32_t height) {
    _backend->onResize(width, height);
}

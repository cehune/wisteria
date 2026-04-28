//
//  Renderer.hpp
//  terrace
//
//  Created by celine on 2026-03-14.
//


#pragma once
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include "backend/IRenderBackend.hpp"

class Renderer {
public:
    Renderer(std::unique_ptr<IRenderBackend> backend);
    void draw(const FrameContext& ctx);
    void onResize(uint32_t width, uint32_t height);
    
    // mouse actions
    void onKey(int key, bool pressed);
    void onScroll(float delta);
    void onMouseDrag(float dx, float dy);
private:
    std::unique_ptr<IRenderBackend> _backend;
};

inline Renderer::Renderer(std::unique_ptr<IRenderBackend> backend)
    : _backend(std::move(backend)) {}

inline void Renderer::draw(const FrameContext& ctx) {
    _backend->draw(ctx);
}

inline void Renderer::onResize(uint32_t width, uint32_t height) {
    _backend->onResize(width, height);
}

inline void Renderer::onKey(int key, bool pressed) {
    _backend->onKey(key, pressed);
}
inline void Renderer::onScroll(float delta) {
    _backend->onScroll(delta);
}
inline void Renderer::onMouseDrag(float dx, float dy) {
    _backend->onMouseDrag(dx, dy);
}

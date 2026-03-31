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
    void draw(MTL::RenderPassDescriptor*, MTL::Drawable*);
private:
    std::unique_ptr<RenderBackend> _backend;
};

inline Renderer::Renderer(std::unique_ptr<RenderBackend> backend)
    : _backend(std::move(backend)) {}

inline void Renderer::draw(MTL::RenderPassDescriptor* desc, MTL::Drawable* drawable) {
    _backend->draw(desc, drawable);
}

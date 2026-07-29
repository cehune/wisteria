//
//  Renderer.cpp
//  wisteria
//
//  Created by celine on 2026-07-27.
//  backend construction and switching
//

#include "Renderer.hpp"
#include "backend/RasterBackend.hpp"
#include "backend/PathTracerBackend.hpp"
#include "platform/scene/Scene.hpp"
#include <iostream>

Renderer::Renderer(MTL::Device* device, Scene* scene, BackendType start)
    : _device(device), _scene(scene) {
    setBackend(start);   // builds and activates the starting backend
}

IRenderBackend* Renderer::ensureBackend(BackendType type) {
    switch (type) {
        case BackendType::Raster:
            if (!_raster) _raster = std::make_unique<RasterBackend>(_device, _scene);
            return _raster.get();
        case BackendType::PathTracer:
            // defer building this
            // BLAS + TLAS take a hot second for complex scenes, so
            // nothing pays for them until the path tracer is actually asked for.
            if (!_pathTracer) _pathTracer = std::make_unique<PathTracerBackend>(_device, _scene);
            return _pathTracer.get();
    }
    return nullptr;   // unreachable; enum is exhaustive
}

void Renderer::setBackend(BackendType type) {
    IRenderBackend* next = ensureBackend(type);
    if (!next) return;

    bool changed = (next != _active);

    // Drain the outgoing backend before the incoming one starts committing
    // ensures no flickering from incoming frame changes
    if (changed && _active) _active->waitIdle();

    _active = next;
    _type   = type;

    // Force a resize in case the size of window changed from the last session
    if (changed && _width && _height) next->onResize(_width, _height);

    // Same reasoning for the camera, need to update it's position view
    // // TODO: Check if shared pointer is better here for camera, just noting that I'm
    // doing the update like this because backends have to build the MTL buffer from it
    // and this tells them to do it (else we still need some other notification I think?)
    if (changed && _hasCamera) next->setCameraState(_cameraState);

    if (changed)
        std::cout << "backend: "
                  << (type == BackendType::Raster ? "raster (edit)" : "path tracer") << "\n";
}

void Renderer::toggleBackend() {
    setBackend(_type == BackendType::Raster ? BackendType::PathTracer : BackendType::Raster);
}

void Renderer::draw(const FrameContext& ctx) { if (_active) _active->draw(ctx); }

void Renderer::onResize(uint32_t width, uint32_t height) {
    _width = width; _height = height;
    if (_active) _active->onResize(width, height);
}

void Renderer::onKey(int key, bool pressed)    { if (_active) _active->onKey(key, pressed); }

// Should be trigerred onto a backend the first time it becomes active. A backend
// built after some camera changes never saw the camera isn't aware of the change
void Renderer::setCameraState(const CameraState& state) {
    _cameraState = state;
    _hasCamera   = true;
    if (_active) _active->setCameraState(state);
}

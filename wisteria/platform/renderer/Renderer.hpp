//
//  Renderer.hpp
//  wisteria
//
//  Created by celine on 2026-03-14.
//  Owns every backend and dispatches to whichever is active. Both backends are kept
//  alive across a switch so toggling back doesn't rebuild the path tracer's
//  PSO/BLAS/TLAS. Contruct complex backends lazily!!
//  Also passes along metal based key inputs
//
#pragma once
#include <Metal/Metal.hpp>
#include <cstdint>
#include <memory>
#include "backend/IRenderBackend.hpp"
#include "RenderConfig.hpp"

class Scene;

class Renderer {
public:
    Renderer(MTL::Device* device, Scene* scene, BackendType start);

    void draw(const FrameContext& ctx);
    void onResize(uint32_t width, uint32_t height);

    // input, forwarded to the active backend
    void onKey(int key, bool pressed);
    void onScroll(float delta);
    void onMouseDrag(float dx, float dy);

    // Backend selection. Public so a UI button or the CLI can drive the switch
    void        setBackend(BackendType type);
    void        toggleBackend();
    BackendType backendType() const { return _type; }
    IRenderBackend* active() const  { return _active; }

private:
    IRenderBackend* ensureBackend(BackendType type);

    MTL::Device* _device = nullptr;
    Scene*       _scene  = nullptr;

    // Both backends outlive any single switch; _active points at one.
    std::unique_ptr<IRenderBackend> _raster;
    std::unique_ptr<IRenderBackend> _pathTracer;
    IRenderBackend* _active = nullptr;
    BackendType     _type   = BackendType::Raster;

    // Last size seen, replayed onto a backend the first time it becomes active.
    uint32_t _width  = 0;
    uint32_t _height = 0;
};

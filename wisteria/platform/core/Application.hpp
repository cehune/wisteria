//
//  Application.hpp
//  wisteria
//
//  Created by celine on 2026-03-12.
//

#pragma once
#include <Metal/Metal.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <iostream>
#include <memory>
#include <string>
#include "Renderer.hpp"
#include "platform/renderer/RenderConfig.hpp"
#include "platform/scene/SceneLoader.hpp"
#include "engine/scene/Camera/Camera.hpp"
#include "engine/scene/Camera/FlyController.hpp"

class Application {
public:
    Application(MTL::Device* device, const RenderConfig& config = {});

    void update();
    void render(const FrameContext& ctx);
    void onResize(uint32_t width, uint32_t height);
    void shutdown();
    void run();

    bool renderOffline();

    // called via MetalView
    void onKey(int key, bool pressed);
    void onScroll(float delta);
    void onMouseDrag(float dx, float dy);

    // passthroughs, user actions to give instruction to the current backend settings
    void        setBackend(BackendType type) { renderer->setBackend(type); }
    BackendType backendType() const          { return renderer->backendType(); }

private:
    void init(const RenderConfig& config);

    // Camera control is only for rasterization right now TODO: confirm
    bool _cameraInputEnabled() const { return renderer->active() && renderer->active()->allowsCameraMovement(); }

    // camera viewpoint is shared among all backends
    Camera                            _camera;
    CameraState                       _cameraState;
    std::unique_ptr<CameraController> _controller;
    
    MTL::Device*              device = nullptr;
    RenderConfig              _config;
    std::unique_ptr<Scene>    scene;      // owns its geometry pool
    std::unique_ptr<Renderer> renderer;   // owns the backends
};

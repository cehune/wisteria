//
//  Application.cpp
//  wisteria
//
//  Created by celine on 2026-03-14.
//

#include "Application.hpp"

Application::Application(MTL::Device* _device, const RenderConfig& config) {
    device = _device;
    init(config);
}

void Application::update() {
    // std::cout << "update";
}
void Application::render(const FrameContext& ctx) {
    _updateCamera(ctx.dt);
    renderer->draw(ctx);
}

void Application::_updateCamera(float dt) {
   
    // I don't want the pathtracers to be able to navigate in real time
    if (renderer->backendType() == BackendType::PathTracer) return;

    // Only pushed when the controller reports real movement
    // TODO: Check if shared pointer is better here for camera
    if (_controller && _controller->update(_cameraState, dt)) {
        renderer->setCameraState(_cameraState);
    }
}

void Application::onResize(uint32_t w, uint32_t h) {
    renderer->onResize(w, h);   // Renderer remembers the size for backend replay
}
void Application::shutdown() {
    // std::cout << "shutdown";
}
void Application::run() {
    // std::cout << "run";
}

void Application::onKey(int key, bool pressed) {
    // Camera Actions
    switch (key) {
        case 13: _controller->onAction(CameraAction::Forward, pressed); return;  // W
        case 0:  _controller->onAction(CameraAction::Left,    pressed); return;  // A
        case 1:  _controller->onAction(CameraAction::Back,    pressed); return;  // S
        case 2:  _controller->onAction(CameraAction::Right,   pressed); return;  // D
        case 14: _controller->onAction(CameraAction::Up,      pressed); return;  // E
        case 12: _controller->onAction(CameraAction::Down,    pressed); return;  // Q
        default: break;
    }
    // Tab toggles switching between backend types
    if (pressed && key == 48) {   // Tab
        renderer->toggleBackend();
        return;
    }
    renderer->onKey(key, pressed);
}
void Application::onScroll(float delta) {
    _controller->onScroll(delta);
}
void Application::onMouseDrag(float dx, float dy) {
    _controller->onMouseDrag(dx, dy);
}

bool Application::renderOffline() {
    // The offline flow sends no resize events, so set size at the start
    // TODO: Enable this for raster
    renderer->onResize(_config.width, _config.height);

    IRenderBackend* backend = renderer->active();
    if (!backend) return false;

    // TODO: setup non progressive (raster) path for offline rendering and remove this check
    IProgressiveRenderer* progressive = backend->asProgressive();
    if (!progressive) {
        std::cerr << "offline render: the active backend can't accumulate samples\n";
        return false;
    }

    const uint32_t spp = _config.targetSamples ? _config.targetSamples : 256;

    std::cout << "offline: " << _config.width << "x" << _config.height
              << " @ " << spp << " spp\n";

    // TODO: consolidate to just a "render" helper for this path
    progressive->renderSamples(spp);
    backend->exportCurrentImage(_config.outPath);
    return true;
}

/* =======PRIVATE=======*/
void Application::init(const RenderConfig& config) {
    _config = config;

    // Fly cam for scene design. Seeded with the framing the path tracer used to
    // hardcode, so switching to it produces the same image as before.
    _cameraState.position = {0.0f, 1.0f, 3.0f};
    _controller = std::make_unique<FlyController>();

    scene = loadScene(device, config.scenePath);
    std::cout << "uploaded all \n";

    renderer = std::make_unique<Renderer>(device, scene.get(), config.backend);

    // Seed the backend before the first frame. Done directly rather than via
    // _updateCamera, which is gated off in path-tracer mode — the offline path
    // starts there and never calls render(), so it would never get a camera.
    _controller->update(_cameraState, 0.0f);
    renderer->setCameraState(_cameraState);
}

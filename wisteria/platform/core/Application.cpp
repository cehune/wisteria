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
    renderer->draw(ctx);
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
    // Tab toggles switching between backend types
    if (pressed && key == 48) {   // Tab
        renderer->toggleBackend();
        return;
    }
    renderer->onKey(key, pressed);
}
void Application::onScroll(float delta) {
    renderer->onScroll(delta);
}
void Application::onMouseDrag(float dx, float dy) {
    renderer->onMouseDrag(dx, dy);
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

    scene = loadScene(device, config.scenePath);
    std::cout << "uploaded all \n";

    renderer = std::make_unique<Renderer>(device, scene.get(), config.backend);
}

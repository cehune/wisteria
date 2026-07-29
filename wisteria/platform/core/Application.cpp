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

/* =======PRIVATE=======*/
void Application::init(const RenderConfig& config) {
    _config = config;

    scene = loadScene(device, config.scenePath);
    std::cout << "uploaded all \n";

    renderer = std::make_unique<Renderer>(device, scene.get(), config.backend);
}

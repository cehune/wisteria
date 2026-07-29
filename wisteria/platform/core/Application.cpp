//
//  Application.cpp
//  wisteria
//
//  Created by celine on 2026-03-14.
//

#include "Application.hpp"
#include "platform/renderer/backend/RasterBackend.hpp"


Application::Application(MTL::Device* _device) {
    device = _device;
    init(device);
}

void Application::update() {
    // std::cout << "update";
}
void Application::render(const FrameContext& ctx) {
    renderer->draw(ctx);
}

void Application::onResize(uint32_t w, uint32_t h) {
    renderer->onResize(w, h);
}
void Application::shutdown() {
    // std::cout << "shutdown";
}

void Application::onKey(int key, bool pressed) {
    renderer->onKey(key, pressed);
}
void Application::onScroll(float delta) {
    renderer->onScroll(delta);
}
void Application::onMouseDrag(float dx, float dy) {
    renderer->onMouseDrag(dx, dy);
}

/* =======PRIVATE=======*/
void Application::init(MTL::Device* device) {
    this->device = device;

    // Cornell box: walls, two blocks, and an emissive ceiling quad. Materials come
    // straight from the MTL (Kd -> albedo, Ke -> area light) — no hardcoded materials.
    scene = loadScene(device);
    std::cout << "uploaded all \n";

    renderer = std::make_unique<Renderer>(makeBackend(_backend));
}

std::unique_ptr<IRenderBackend> Application::makeBackend(BackendType type) {
    switch (type) {
        case BackendType::Raster:
            return std::make_unique<RasterBackend>(device, scene.get());
        case BackendType::PathTracer:
            return std::make_unique<PathTracerBackend>(device, scene.get());
    }
    return nullptr; // unreachable; enum is exhaustive
}

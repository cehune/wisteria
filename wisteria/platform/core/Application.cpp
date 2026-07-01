//
//  Application.cpp
//  wisteria
//
//  Created by celine on 2026-03-14.
//

#include "Application.hpp"
#include "platform/renderer/backend/RasterBackend.hpp"
#include "engine/io/AssetPath.hpp"

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
    pool     = std::make_unique<SceneGeometryPool>();
    scene    = std::make_unique<Scene>(*pool);
    const std::string teapot = wisteria::assets::samplePath("utah_teapot.obj");

    auto translation = [](float x, float y, float z) {
        simd::float4x4 m = matrix_identity_float4x4;
        m.columns[3] = {x, y, z, 1.0f};
        return m;
    };
    // three Lambertian materials so the per-instance material path is visible
    uint32_t matRed   = scene->addMaterial({ MATERIAL_LAMBERTIAN, { 0.85f, 0.25f, 0.25f } });
    uint32_t matGreen = scene->addMaterial({ MATERIAL_LAMBERTIAN, { 0.30f, 0.75f, 0.35f } });
    uint32_t matBlue  = scene->addMaterial({ MATERIAL_LAMBERTIAN, { 0.30f, 0.45f, 0.85f } });

    scene->addMeshInstance(teapot, translation(-3.0f, 0.0f, 0.0f), device, matRed);
    scene->addMeshInstance(teapot, translation( 0.0f, 0.0f, 0.0f), device, matGreen);
    scene->addMeshInstance(teapot, translation( 3.0f, 0.0f, 0.0f), device, matBlue);

    // Build the GPU mega buffers from everything staged above.
    pool->finalize();
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

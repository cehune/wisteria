//
//  Application.cpp
//  terrace
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

/* =======PRIVATE=======*/
void Application::init(MTL::Device* device) {
    this->device = device;
    scene    = std::make_unique<Scene>();
        
    // Hardcode a triangle for now
    Mesh mesh{};
    mesh.index = 0;
    mesh.numTriangles = 1;
    
    std::vector<Vertex> verts = {
        {{ 0.0f,       0.577f, 0.0f, 1.0f}, {}, {}, {}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{-0.5f,      -0.289f, 0.0f, 1.0f}, {}, {}, {}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{ 0.5f,      -0.289f, 0.0f, 1.0f}, {}, {}, {}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };
    
    std::vector<uint32_t> indices = {0, 1, 2};
    scene->addMeshDirect(mesh, verts, indices, device);

    std::cout << "uploaded all \n";
    
    auto backend = std::make_unique<PathTracerBackend>(device, scene.get());
    renderer = std::make_unique<Renderer>(std::move(backend));
}

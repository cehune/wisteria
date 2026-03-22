//
//  Application.cpp
//  terrace
//
//  Created by celine on 2026-03-14.
//

#include "Application.hpp"

Application::Application(MTL::Device* _device) {
    device = _device;
    init(device);
}

void Application::update() {
    // std::cout << "update";
}
void Application::render(MTL::RenderPassDescriptor* desc,
                         MTL::Drawable* drawable) {
    renderer->draw(desc, drawable);
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
        {{ 0.0f,  0.5f, 0.0f, 1.0f}, {}, {}, {}, {1.0f, 0.0f, 0.0f, 1.0f}},
        {{-0.5f, -0.5f, 0.0f, 1.0f}, {}, {}, {}, {0.0f, 1.0f, 0.0f, 1.0f}},
        {{ 0.5f, -0.5f, 0.0f, 1.0f}, {}, {}, {}, {0.0f, 0.0f, 1.0f, 1.0f}},
    };
    
    std::vector<uint32_t> indices = {0, 1, 2};
    scene->addMeshDirect(mesh, verts, indices, device);

    std::cout << "uploaded all \n";
    
    renderer = std::make_unique<Renderer>(device, scene.get());
}

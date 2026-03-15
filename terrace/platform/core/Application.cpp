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
    commandQueue = device->newCommandQueue();
    renderer = std::make_unique<Renderer>(device);
}

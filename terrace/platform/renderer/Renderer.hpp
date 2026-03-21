//
//  Renderer.hpp
//  terrace
//
//  Created by celine on 2026-03-14.
//


#pragma once
#include <Metal/Metal.hpp>

class Renderer {
public:
    Renderer(MTL::Device* _device);
    void draw(MTL::RenderPassDescriptor* desc, MTL::Drawable* drawable) ;
    
private:
    MTL::Device* device; // Application will own the device, renderer will just have reference
    // note - can't use unique ptr for mtl objects - they have to call release not delete
};

//
//  PipelineLibrary.cpp
//  terrace
//
//  Created by celine on 2026-03-21.
//
#include "PipelineLibrary.hpp"

MTL::RenderPipelineState* PipelineLibrary::getOrCreate(const PipelineKey& key) {
    auto it = _cache.find(key);
        if (it != _cache.end()) return it->second;
    // convert pipelinekey shader functions to NS compatible type
    MTL::Function* vertFn = resolveFunction(key.vertID);
    MTL::Function* fragFn = resolveFunction(key.fragID);
    
    // set pipeline descriptor with shaders
    MTL::RenderPipelineDescriptor* desc = MTL::RenderPipelineDescriptor::alloc()->init();
    desc->setVertexFunction(vertFn);
    desc->setFragmentFunction(fragFn);
    desc->colorAttachments()->object(0)->setPixelFormat(toMTL(key.pixelFormat));

    NS::Error* err = nullptr;
    auto* pso = _device->newRenderPipelineState(desc, &err);

    desc->release();

    assert(pso && "Pipeline compilation failed — check shader function names in .metal");

    _cache[key] = pso;
    return pso;
}

MTL::PixelFormat PipelineLibrary::toMTL(PixelFormat fmt) {
    switch (fmt) {
        case PixelFormat::BGRA8Unorm:  return MTL::PixelFormatBGRA8Unorm;
        case PixelFormat::RGBA16Float: return MTL::PixelFormatRGBA16Float;
        default: return MTL::PixelFormatBGRA8Unorm;
    }
}

MTL::Function* PipelineLibrary::resolveFunction(uint32_t id) {
    auto it = _shaderCache.find(id);
    assert(it != _shaderCache.end() && "Unregistered shader ID — call registerShader first");
    return it->second;
}

uint32_t PipelineLibrary::registerShader(const std::string& fnName) {
    // validate against the Metal library immediately
    auto* fn = _library->newFunction(
        NS::String::string(fnName.c_str(), NS::UTF8StringEncoding));

    assert(fn && "Shader function not found — check spelling and target membership");

    uint32_t id = static_cast<uint32_t>(_idToName.size());
    _idToName.push_back(fnName);
    _nameToID[fnName] = id;
    _shaderCache[id] = fn;   // pre-populate stage 2 cache while we have fn
    return id;
}

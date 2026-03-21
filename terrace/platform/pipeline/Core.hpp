//
//  pipeline/Core.hpp
//  terrace
//
//  Created by celine on 2026-03-21.
//
#pragma once
#include <string>
#include <unordered_map>
#include <Metal/Metal.hpp>

struct PipelineKey {
    std::string vertFn;
    std::string fragFn;
    MTL::PixelFormat pixelFormat;
    bool operator==(const PipelineKey&) const;
};

class PipelineLibrary {
public:
    PipelineLibrary(MTL::Device*, MTL::Library*);
    MTL::RenderPipelineState* getOrCreate(const PipelineKey&);
private:
    // All potential varients, each mesh should reference a pipelinekey
    std::unordered_map<PipelineKey, MTL::RenderPipelineState*> _cache;
};
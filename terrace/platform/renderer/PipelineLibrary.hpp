//
//  PipelineLibrary.hpp
//  terrace
//
//  Created by celine on 2026-03-21.
//
// Uses the pipelinekey in engine/renderer/pipelinekey, converts generic pixel
// format to MTL::PixelFormat
// General class for storing MTL functions

/*
 PipelineLibrary — shader and pipeline state cache
 
 Three-stage, but runs at most once per target
 
 STAGE 1 — registerShader(name)  [string → uint32_t]
    Validates the name exists in the Metal library immediately — fails loud
    at init time rather than silently at draw time. Pre-populates the function
    cache so stage 2 is always a guaranteed hit. String hashing occurs once here
    and never again.

  STAGE 2 — resolveFunction(id)  [uint32_t → MTL::Function*]
    Pure cache lookup after registerShader pre-population. Unreachable miss
    path is guarded by assert. No Metal calls in valid usage.

 STAGE 3 — getOrCreate(PipelineKey)  [PipelineKey → MTL::RenderPipelineState*]
   PipelineKey contains only uint32_t IDs and an enum — all integer types.
   Hashing is cheap integer arithmetic, no string involvement.
   On cache miss: resolves functions via stage 2, links the full pipeline state.
   Pipeline state compilation is expensive (~50-200ms) but happens once per
   unique {vertID, fragID, pixelFormat} combination.
   Cost: integer hash on every call, Metal pipeline compile on first call only.

 The key invariant: by the time we are in the hot path (buildRenderList each
 frame), all pipelines are already compiled and getOrCreate is a pure integer
 hash lookup into _cache. No string hashing, no Metal calls, no allocations.

 _nameToID      std::unordered_map<string, uint32_t>   name → stable ID
 _idToName      std::vector<string>                    ID → name (vector index)
 _shaderCache   std::unordered_map<uint32_t, MTL::Function*>   ID → compiled function
 _cache         std::unordered_map<PipelineKey, MTL::RenderPipelineState*>
 
 registerShader("vertex_main")
   → assigns ID 0
   → stored in _nameToID["vertex_main"] = 0
   → stored in _shaderCache[0] = MTL::Function*

 registerPipeline("vertex_main", "fragment_main")
   → calls registerShader for each name, gets back IDs 0 and 1
   → constructs PipelineKey { vertID=0, fragID=1, fmt=BGRA8 }
   → stores key in _pipelines[0]
   → returns PipelineID 0 to caller

 meshInstance.pipelineID = 0

 buildRenderList each frame
   → _pipelines[meshInstance.pipelineID]    → PipelineKey { 0, 1, BGRA8 }
   → _cache[PipelineKey { 0, 1, BGRA8 }]   → MTL::RenderPipelineState*
   → _shaderCache[0]                        → MTL::Function* (only on cache miss)
*/

#include <Metal/Metal.hpp>
#include "engine/renderer/PipelineKey.hpp"

class PipelineLibrary {
public:
    MTL::RenderPipelineState* getOrCreate(const PipelineKey&);
    MTL::Function* getShaderFunction(const std::string& name);
    uint32_t registerShader(const std::string& fnName);

private:
    MTL::PixelFormat toMTL(PixelFormat fmt);
    MTL::Function* resolveFunction(uint32_t id);
    
    // engine to here
    std::unordered_map<std::string, uint32_t>  _nameToID;
    std::vector<std::string>                   _idToName;

    // caches
    std::unordered_map<uint32_t, MTL::Function*> _shaderCache;
    std::unordered_map<PipelineKey, MTL::RenderPipelineState*> _cache;
    
    MTL::Library* _library;
    MTL::Device * _device;
};

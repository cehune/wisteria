//
//  PathTracerBackend.cpp
//  terrace
//
//  Created by celine on 2026-03-31.
//

#include "PathTracerBackend.hpp"

PathTracerBackend::PathTracerBackend(MTL::Device* device, Scene* scene): _device(device), _scene(scene) {
    _commandQueue = device->newCommandQueue();
    _buildPipeline();
    _updateCameraBuffer();
    std::cout << "Pathtracer backend setup done";
}

PathTracerBackend::~PathTracerBackend() {
    if (_pso)       _pso->release();
    if (_offscreen) _offscreen->release();
    _commandQueue->release();
}

void PathTracerBackend::_buildPipeline() {
    NS::Error* err = nullptr;
    MTL::Library* lib = _device->newDefaultLibrary();
    assert(lib && "default.metallib not found");

    MTL::Function* fn = lib->newFunction(
        NS::String::string("raytrace_kernel", NS::UTF8StringEncoding));
    assert(fn && "raytrace_kernel not found");

    _pso = _device->newComputePipelineState(fn, &err);
    assert(_pso && "failed to build compute PSO");

    fn->release();
    lib->release();
}


void PathTracerBackend::_buildOffscreenTexture(uint32_t w, uint32_t h) {
    if (_offscreen) _offscreen->release();

    MTL::TextureDescriptor* td = MTL::TextureDescriptor::texture2DDescriptor(
        MTL::PixelFormatRGBA8Unorm, w, h, false);
    td->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
    td->setStorageMode(MTL::StorageModePrivate);  // GPU-only, blit to drawable after

    _offscreen = _device->newTexture(td);
    _width  = w;
    _height = h;
}

void PathTracerBackend::_updateCameraBuffer() {
    if (!_cameraBuffer) {
        // TODO: check if resource mode shared is sufficient for now?
        _cameraBuffer = _device->newBuffer(
            &_camera, sizeof(CameraUniformsPT), MTL::ResourceStorageModeShared
        );
    } else {
        memcpy(_cameraBuffer->contents(), &_camera, sizeof(CameraUniformsPT));
    }
}

void PathTracerBackend::onResize(uint32_t w, uint32_t h) {
    _buildOffscreenTexture(w, h);
    // camera object actions
    _width = w;
    _height = h;
    _updateCameraBuffer();
}

// ── Per-frame trace ───────────────────────────────────────────────────────────

void PathTracerBackend::draw(const FrameContext& ctx) {
    // Lazily build offscreen texture on first call
    if (!_offscreen) {
        _buildOffscreenTexture(ctx.width, ctx.height);
    }

    SceneGeometryPool& pool = _scene->geometryPool();
    MTL::CommandBuffer* cmd = _commandQueue->commandBuffer();
    MTL::ComputeCommandEncoder* enc = cmd->computeCommandEncoder();

    enc->setComputePipelineState(_pso);
    
    
    uint32_t totalTri = 0;
    for (Mesh& m : _scene->meshes()) totalTri += m.numTriangles;

    MTL::Buffer* vb = pool.mergeVertexBuffer(_device);
    MTL::Buffer* ib = pool.mergeIndexBuffer(_device);
    MTL::Buffer* numTriBuf = _device->newBuffer(&totalTri, sizeof(uint32_t), MTL::ResourceStorageModeShared);

    enc->setTexture(_offscreen, 0);
    enc->setBuffer(vb,        0, 0);
    enc->setBuffer(ib,        0, 1);
    enc->setBuffer(numTriBuf, 0, 2);
    enc->setBuffer(_cameraBuffer, 0, 3);
    
    NS::UInteger tw = _pso->threadExecutionWidth();
    NS::UInteger th = _pso->maxTotalThreadsPerThreadgroup() / tw;
    MTL::Size threadsPerGroup = MTL::Size::Make(tw, th, 1);
    MTL::Size threadsPerGrid  = MTL::Size::Make(_width, _height, 1);

    enc->dispatchThreads(threadsPerGrid, threadsPerGroup);
    enc->endEncoding();
    
    vb->release();
    ib->release();
    numTriBuf->release();

    CA::MetalDrawable* metalDrawable = static_cast<CA::MetalDrawable*>(ctx.drawable);
    MTL::BlitCommandEncoder* blit = cmd->blitCommandEncoder();
    blit->copyFromTexture(_offscreen, metalDrawable->texture());
    blit->endEncoding();

    cmd->presentDrawable(ctx.drawable);
    cmd->commit();
}

//
//  PathTracerBackend.cpp
//  terrace
//
//  Created by celine on 2026-03-31.
//

#include "PathTracerBackend.hpp"

PathTracerBackend::PathTracerBackend(MTL::Device* device, Scene* scene): _device(device), _scene(scene) {
    // TEMP
    _currentCameraState = CameraState();
    _currentCameraState.position = {0.0f,2.0f,15.0f};
    
    _commandQueue = device->newCommandQueue();
    _buildPipeline();
    _updateCameraBuffer();
    _dirty = true; // accumulation buffer
    
    std::cout << "Pathtracer backend setup done";
}

PathTracerBackend::~PathTracerBackend() {
    if (_pso)          _pso->release();
    if (_offscreen)    _offscreen->release();
    if (_accumulation) _accumulation->release();
    if (_cameraBuffer) _cameraBuffer->release();
    if (_commandQueue) _commandQueue->release();
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
    td->setUsage(MTL::TextureUsageShaderWrite);
    td->setStorageMode(MTL::StorageModePrivate);  // GPU-only, blit to drawable after

    _offscreen = _device->newTexture(td);
}

void PathTracerBackend::_buildAccumulationTexture(uint32_t w, uint32_t h) {
    if (_accumulation) _accumulation->release();
    
    MTL::TextureDescriptor* td = MTL::TextureDescriptor::texture2DDescriptor(
       MTL::PixelFormatRGBA32Float, w, h, false);
    td->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
    td->setStorageMode(MTL::StorageModePrivate); // only for the GPU, use private
    
    _accumulation = _device->newTexture(td);
}

void PathTracerBackend::_updateCameraBuffer() {
    // set the uniforms based on the current camera state
    // get right, up, and fwd vectors via quat
    _cameraUniforms.right = _camera.right(_currentCameraState);
    _cameraUniforms.up = _camera.up(_currentCameraState);
    _cameraUniforms.forward = _camera.forward(_currentCameraState);
    _cameraUniforms.origin = _currentCameraState.position;
    _cameraUniforms.fov = _currentCameraState.fov;

    if (!_cameraBuffer) {
        // TODO: check if resource mode shared is sufficient for now?
        _cameraBuffer = _device->newBuffer(
            &_cameraUniforms, sizeof(CameraUniformsPT), MTL::ResourceStorageModeShared
        );
    } else {
        memcpy(_cameraBuffer->contents(), &_cameraUniforms, sizeof(CameraUniformsPT));
    }
}

void PathTracerBackend::onResize(uint32_t w, uint32_t h) {
    _buildOffscreenTexture(w, h);
    // camera object actions
    _width = w;
    _height = h;
    _updateCameraBuffer();

    // Mark accumulation dirty — the draw loop will rebuild the texture and
    // reset _sampleCount on the next frame. Avoids rebuilding twice when
    // onResize is followed immediately by draw.
    _dirty = true;
}

// ── Per-frame trace ───────────────────────────────────────────────────────────

void PathTracerBackend::draw(const FrameContext& ctx) {
    // Lazily build offscreen texture on first call
    if (!_offscreen) {
        _buildOffscreenTexture(ctx.width, ctx.height);
        _width = ctx.width;
        _height = ctx.height;
    }
    // remake the accumulation buffer if it was dirty
    if (_dirty) {
        _buildAccumulationTexture(ctx.width, ctx.height);
        _dirty = false;
        _sampleCount = 0;
    }
    
    IGeometryPool& pool = _scene->geometryPool();
    MTL::CommandBuffer* cmd = _commandQueue->commandBuffer();
    MTL::ComputeCommandEncoder* enc = cmd->computeCommandEncoder();
    // pso is built from buildPipeline based on a given metal shader
    enc->setComputePipelineState(_pso);

    // total triangles for set of all meshes
    uint32_t totalTri = 0;
    // TODO: use mesh instances once we switch to proper tracing
    for (Mesh& m : _scene->meshes()) totalTri += m.numTriangles;

    MTL::Buffer* vb = pool.vertexBuffer();
    MTL::Buffer* ib = pool.indexBuffer();

    enc->setTexture(_offscreen, 0);
    enc->setTexture(_accumulation, 1);
    enc->setBuffer(vb,            0, 0);
    enc->setBuffer(ib,            0, 1);
    enc->setBytes(&totalTri, sizeof(uint32_t), 2);
    enc->setBuffer(_cameraBuffer, 0, 3);
    enc->setBytes(&_sampleCount, sizeof(uint32_t), 4);

    // run the kernel function per pixel per thread, (makes as many threads as pixels)
    NS::UInteger tw = _pso->threadExecutionWidth();
    NS::UInteger th = _pso->maxTotalThreadsPerThreadgroup() / tw;
    MTL::Size threadsPerGroup = MTL::Size::Make(tw, th, 1);
    MTL::Size threadsPerGrid  = MTL::Size::Make(_width, _height, 1);

    enc->dispatchThreads(threadsPerGrid, threadsPerGroup);
    enc->endEncoding();
    ++_sampleCount;
    
    // kinda like dma, need to copy from off screen texture to actual texture
    CA::MetalDrawable* metalDrawable = static_cast<CA::MetalDrawable*>(ctx.drawable);
    MTL::BlitCommandEncoder* blit = cmd->blitCommandEncoder();
    blit->copyFromTexture(_offscreen, metalDrawable->texture());
    blit->endEncoding();

    // schedules to appear in the window
    cmd->presentDrawable(ctx.drawable);
    cmd->commit();
}

void PathTracerBackend::setCameraState(const CameraState& state) {
    _currentCameraState.far = state.far;
    _currentCameraState.near = state.near;
    _currentCameraState.fov = state.fov;
    _currentCameraState.position = state.position;
    _currentCameraState.orientation = state.orientation;

    // Push the new uniforms to the GPU and invalidate the accumulator —
    // otherwise old samples from the previous view ghost into the new one.
    _updateCameraBuffer();
    _dirty = true;
}

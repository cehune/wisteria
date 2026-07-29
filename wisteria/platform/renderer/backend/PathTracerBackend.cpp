//
//  PathTracerBackend.cpp
//  wisteria
//
//  Created by celine on 2026-03-31.
//

#include "PathTracerBackend.hpp"
#include "engine/io/AssetPath.hpp"
#include <algorithm>

PathTracerBackend::PathTracerBackend(MTL::Device* device, Scene* scene): _device(device), _scene(scene) {
    // TEMP
    _drawBufferSemaphore = dispatch_semaphore_create(_maxBuffers);
    _currentCameraState = CameraState();
    _currentCameraState.position = {0.0f, 1.0f, 3.0f};   // outside the Cornell box, looking -z
    
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
    if (_moment2)      _moment2->release();
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
        MTL::PixelFormatBGRA8Unorm, w, h, false);
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

    // Welford second moment of luminance/ Same size and lifetime as _accumulation,
    // so it rebuilds and resets here in lockstep.
    if (_moment2) _moment2->release();

    MTL::TextureDescriptor* mtd = MTL::TextureDescriptor::texture2DDescriptor(
       MTL::PixelFormatR32Float, w, h, false);
    mtd->setUsage(MTL::TextureUsageShaderWrite | MTL::TextureUsageShaderRead);
    mtd->setStorageMode(MTL::StorageModePrivate);

    _moment2 = _device->newTexture(mtd);
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
    if (ctx.width && ctx.height && (ctx.width != _width || ctx.height != _height)) {
        onResize(ctx.width, ctx.height);
    }

    if (_maxSamples == 0 || _sampleCount < _maxSamples) renderSamples(1);
    if (ctx.drawable) presentRender(ctx);
}

// Accumulates n samples into _accumulation. Touches no drawable and reads
// nothing off the FrameContext. 
// everything sizes from _width/_height, from onResize().
// We want this decoupling so production doesn't run at the display's refresh rate.
// n is how many samples we want to add RIGHT NOW while sampleCount is the total
void PathTracerBackend::renderSamples(uint32_t n) {
    if (_width == 0 || _height == 0) return;

    // Lazily build offscreen texture on first call
    if (!_offscreen) _buildOffscreenTexture(_width, _height);

    // remake the accumulation buffer if it was dirty
    if (_dirty) {
        _buildAccumulationTexture(_width, _height);
        _dirty = false;
        _sampleCount = 0;
    }

    // Build BLAS/TLAS + material buffer once, lazily (geometry is finalized by first draw).
    if (!_accelBuilt) {
        _accel.build(_device, _commandQueue, *_scene);
        _scene->buildMaterialBuffer(_device);
        _scene->buildLightBuffer(_device);
        _accelBuilt = true;
    }

    // Loop-invariant bindings and dispatch geometry, hoisted out of the
    // per-sample loop below.
    IGeometryPool& pool = _scene->geometryPool();
    MTL::Buffer* vb = pool.vertexBuffer();
    MTL::Buffer* ib = pool.indexBuffer();
    uint32_t numLights = _scene->numLights();

    // run the kernel function per pixel per thread, (makes as many threads as pixels)
    NS::UInteger tw = _pso->threadExecutionWidth();
    NS::UInteger th = _pso->maxTotalThreadsPerThreadgroup() / tw;
    MTL::Size threadsPerGroup = MTL::Size::Make(tw, th, 1);
    MTL::Size threadsPerGrid  = MTL::Size::Make(_width, _height, 1);

    for (uint32_t i = 0; i < n; ++i) {
        dispatch_semaphore_wait(_drawBufferSemaphore, DISPATCH_TIME_FOREVER);
        MTL::CommandBuffer* cmd = _commandQueue->commandBuffer();
        // max of _maxBuffers cmd buffers allowed to be queued at a time
        cmd->addCompletedHandler([this](MTL::CommandBuffer*) {
            dispatch_semaphore_signal(_drawBufferSemaphore);
        });

        MTL::ComputeCommandEncoder* enc = cmd->computeCommandEncoder();
        // pso is built from buildPipeline based on a given metal shader
        enc->setComputePipelineState(_pso);

        enc->setTexture(_offscreen, 0);
        enc->setTexture(_accumulation, 1);
        enc->setTexture(_moment2, 2);   // R32F Welford m2 — must match moment2Tex [[texture(2)]]
        enc->setBuffer(vb,            0, 0);   // mega VB — for normal re-fetch on hit
        enc->setBuffer(ib,            0, 1);   // mega IB
        // buffer(2) (triangle count) retired — traversal is via the TLAS now.
        enc->setBuffer(_cameraBuffer, 0, 3);
        // Re-bound every iteration: _sampleCount advances inside this loop, and
        // the kernel's first-sample branch keys on it being 0.
        enc->setBytes(&_sampleCount, sizeof(uint32_t), 4);

        // TLAS (buffer 5) + per-instance shading payload (buffer 6) for the kernel.
        // Residency: a TLAS does NOT keep its BLASes resident, so mark each used.
        if (_accel.tlas()) {
            enc->setAccelerationStructure(_accel.tlas(), 5);
            enc->setBuffer(_accel.instanceData(), 0, 6);
            for (MTL::AccelerationStructure* b : _accel.blases())
                if (b) enc->useResource(b, MTL::ResourceUsageRead);
        }
        enc->setBuffer(_scene->materialBuffer(), 0, 7);   // per-instance materials

        // area lights (buffer 8) + count (buffer 9). A light-less scene binds nothing
        // at 8, so the kernel gates all light reads on numLights > 0.
        if (_scene->lightBuffer()) enc->setBuffer(_scene->lightBuffer(), 0, 8);
        enc->setBytes(&numLights, sizeof(uint32_t), 9);

        enc->dispatchThreads(threadsPerGrid, threadsPerGroup);
        enc->endEncoding();
        cmd->commit();
        ++_sampleCount;
    }
}

// Blit-only: the kernel already wrote the sRGB-encoded image into _offscreen in the compute shader
void PathTracerBackend::presentRender(const FrameContext& ctx) {
    if (!ctx.drawable || !_offscreen) return;

    CA::MetalDrawable* metalDrawable = static_cast<CA::MetalDrawable*>(ctx.drawable);
    MTL::Texture* drawableTex = metalDrawable->texture();
    std::cout << "off=" << _offscreen->width() << "x" << _offscreen->height()
          << "  draw=" << drawableTex->width() << "x" << drawableTex->height() << "\n";

    if (!drawableTex) return;

    // Mid-drag the drawable can already be a size ahead of _offscreen. 
    // Skip the frame since the next one comes after onResize anyway.
    if (_offscreen->width()  != drawableTex->width() ||
        _offscreen->height() != drawableTex->height()) return;

    // kinda like dma, need to copy from off screen texture to actual texture
    MTL::CommandBuffer* cmd = _commandQueue->commandBuffer();
    MTL::BlitCommandEncoder* blit = cmd->blitCommandEncoder();
    blit->copyFromTexture(_offscreen, drawableTex);
    blit->endEncoding();

    // schedules to appear in the window
    cmd->presentDrawable(ctx.drawable);
    cmd->commit();
}

void PathTracerBackend::continueTo(uint32_t newTarget) {
       _maxSamples = newTarget;   // draw()'s halt check resumes automatically next frame
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

void PathTracerBackend::onKey(int key, bool pressed) {
    if (pressed && key == 1) exportCurrentImage("");   // S = save
}

void PathTracerBackend::exportCurrentImage(const std::string& path) {
    std::vector<float> pixels;
    uint32_t w = 0, h = 0;
    if (!_readbackAccumulationTexture(pixels, w, h)) {
        std::cerr << "exportCurrentImage: export failed, likely empty accumulation buffer";
        return;
    }

    // Alpha is the per-pixel sample count N, just use min and max N to log the results for sanity
    float minN = pixels[3], maxN = pixels[3];
    for (size_t i = 3; i < pixels.size(); i += 4) {
        minN = std::min(minN, pixels[i]);
        maxN = std::max(maxN, pixels[i]);
    }

    std::string name = "wisteria_" + std::to_string(_sampleCount) + "spp.pfm";
    std::filesystem::path filepath;
    if (!path.empty()) {
        filepath = path;                       // explicit path wins, used as given
    } else {
        // Land in <repo>/outputs/pfm
        std::string out = wisteria::assets::outputPath(name);
        filepath = out.empty() ? std::filesystem::path(name) : std::filesystem::path(out);
    }

    // Append rather than replace_extension(): replacing would turn    
    if (filepath.extension() != ".pfm") {
        filepath += ".pfm";
    }

    if (wisteria::pfm::writePFM(filepath.string(), pixels.data(), w, h)) {
        std::cout << "Exported " << filepath.string()
                  << " (" << w << "x" << h
                  << ", ~" << _sampleCount << " spp encoded, per-pixel N "
                  << minN << ".." << maxN << ")\n";
    } else {
        std::cerr << "exportCurrentImage: failed to write file to " << filepath.string() << "\n";
    }
}


// Blits _accumulation (StorageModePrivate) into a Shared staging buffer and
// copies it into `out` as tightly packed RGBA32F.
bool PathTracerBackend::_readbackAccumulationTexture(std::vector<float>& out,
                                                     uint32_t& outW, uint32_t& outH) {
    if (!_accumulation) return false;

    // don't use _width and _height because theres a moment between onResize and the dirty flag where accumulation isn't rebuilt
    outW = static_cast<uint32_t>(_accumulation->width());
    outH = static_cast<uint32_t>(_accumulation->height());

    const NS::UInteger bytesPerRow = outW * 4 * sizeof(float);
    const NS::UInteger byteCount = bytesPerRow * outH;

    // need to convert the mtl accum texture to a buffer
    MTL::Buffer* accumBuffer =  _device->newBuffer(byteCount, MTL::ResourceStorageModeShared);
    if (!accumBuffer) return false;

    MTL::CommandBuffer* cmd = _commandQueue->commandBuffer();
    MTL::BlitCommandEncoder* blit = cmd->blitCommandEncoder();
    blit->copyFromTexture(_accumulation, 0, 0, MTL::Origin(0,0,0),
                          MTL::Size(_width, _height, 1), accumBuffer,
                          0, bytesPerRow, byteCount);
    blit->endEncoding();
    cmd->commit();
    cmd->waitUntilCompleted();

    out.resize(static_cast<size_t>(outW) * outH * 4);
    memcpy(out.data(), accumBuffer->contents(), byteCount);
    accumBuffer->release();
    return true;
}

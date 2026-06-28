//
//  RasterBackend.cpp
//  wisteria
//
//  Created by celine on 2026-03-30.
//

#include "RasterBackend.hpp"

RasterBackend::~RasterBackend() {
    if (_depthState) _depthState->release();
    for (auto& buf: _cameraBuffers) {
        if (buf) {
            buf->release();
            buf = nullptr;
        }
    }
}

RasterBackend::RasterBackend(MTL::Device* device, Scene* scene): _device(device), _scene(scene), _aspect(800.0f/600.0f) {
    
    _camBufferSemaphore = dispatch_semaphore_create(_maxBuffers);
    _commandQueue = device->newCommandQueue();
    // Load default .metallib
    MTL::Library* lib = device->newDefaultLibrary();
    
    _pipelineLibrary = std::make_unique<PipelineLibrary>(device, lib);
    
    // Register shaders, get IDs
    uint32_t vertID = _pipelineLibrary->registerShader("vertex_main");
    uint32_t fragID = _pipelineLibrary->registerShader("fragment_main");
    
    // Build and cache the pipeline
    PipelineKey key{ vertID, fragID, PixelFormat::BGRA8Unorm };
    _pipelineLibrary->getOrCreate(key);
    _pipelines.push_back(key);
    
    lib->release();
    
    // Depth-stencil state: closer fragments win, and we write depth so later
    // draws test against it. Needed now that instances overlap in the scene.
    MTL::DepthStencilDescriptor* dsd = MTL::DepthStencilDescriptor::alloc()->init();
    dsd->setDepthCompareFunction(MTL::CompareFunctionLess);
    dsd->setDepthWriteEnabled(true);
    _depthState = device->newDepthStencilState(dsd);
    dsd->release();

    // Camera controller: fly cam for scene design (swap to OrbitController to orbit).
    _cameraController = std::make_unique<FlyController>();

    std::cout << "rasterizer setup all done \n";
    
    //_currentCameraState.position = {0,0,20};
}

void RasterBackend::draw(const FrameContext& ctx) {
    // build camera context and projection matrices
    dispatch_semaphore_wait(_camBufferSemaphore, DISPATCH_TIME_FOREVER); // prevent concurrent read / write to same cam buffer
    MTL::CommandBuffer* cmd = _commandQueue->commandBuffer();
    MTL::RenderCommandEncoder* enc = cmd->renderCommandEncoder(ctx.renderPassDesc);
    
    // set the pipeline state
    PipelineKey key = _pipelines[0];
    MTL::RenderPipelineState* pso = _pipelineLibrary->getOrCreate(key);
    if (!pso) {
        std::cerr << "ERROR: pipeline state is null\n";
        return;
    }
    enc->setRenderPipelineState(pso);
    enc->setDepthStencilState(_depthState);
    
    // camera movement based on input actions
    _currentCameraState = _cameraController->update(_currentCameraState, ctx.dt);

    // Advance the triple-buffer ring, then write this frame's camera into the
    // slot the GPU isn't reading
    _frameIndex = (_frameIndex + 1) % _maxBuffers;
    _updateCameraBuffer();

    enc->setVertexBuffer(_cameraBuffers[_frameIndex], 0, 1); // camera
    cmd->addCompletedHandler([this](MTL::CommandBuffer*) {
        dispatch_semaphore_signal(_camBufferSemaphore);
    });
    
    // retrieve pool — single megabuffer for all unique meshes
    IGeometryPool& pool = _scene->geometryPool();
    MTL::Buffer* megaVB = pool.vertexBuffer();
    MTL::Buffer* megaIB = pool.indexBuffer();

    enc->setVertexBuffer(megaVB, 0, 0);

    for (const MeshInstance& inst : _scene->instances()) {
        // transforms (model matrix) fed into buffer per instance (one transform for whole buffer)
        enc->setVertexBytes(&inst.transform, sizeof(inst.transform), 2);
        enc->drawIndexedPrimitives(
            MTL::PrimitiveTypeTriangle,
            static_cast<NS::UInteger>(inst.indexCount),
            MTL::IndexTypeUInt32,
            megaIB,
            static_cast<NS::UInteger>(inst.indexOffset) * sizeof(uint32_t));
    }
        
    enc->endEncoding();
    cmd->presentDrawable(ctx.drawable);
    cmd->commit();
}

void RasterBackend::onResize(uint32_t width, uint32_t height) {
    // TODO: handle resize
    _width = width;
    _height = height;
    _aspect = (float)width / (float)height;
    _updateCameraBuffer();
}

// Private methods
void RasterBackend::_updateCameraBuffer() {
    // camera uniforms is for the shaders, camera class is for actual object data
    // create a buffer for the shaders thats just the curretn camera unifroms
    CameraUniformsRaster cam;
    cam.viewProjection = simd_mul(_camera.projectionMatrix(_currentCameraState, _aspect), _camera.viewMatrix(_currentCameraState));
    
    if (!_cameraBuffers[_frameIndex]) {
        _cameraBuffers[_frameIndex] = _device->newBuffer(&cam, sizeof(CameraUniformsRaster), MTL::ResourceStorageModeShared);
    } else { // memcopy if avail
        memcpy(_cameraBuffers[_frameIndex]->contents(), &cam, sizeof(CameraUniformsRaster));
    }
}

void RasterBackend::onKey(int key, bool pressed) {
    // Platform input mapping lives here (platform layer), not in the engine
    // controllers: translate macOS ANSI key codes into semantic camera actions.
    switch (key) {
        case 13: _cameraController->onAction(CameraAction::Forward, pressed); break; // W
        case 0:  _cameraController->onAction(CameraAction::Left,    pressed); break; // A
        case 1:  _cameraController->onAction(CameraAction::Back,    pressed); break; // S
        case 2:  _cameraController->onAction(CameraAction::Right,   pressed); break; // D
        case 14: _cameraController->onAction(CameraAction::Up,      pressed); break; // E
        case 12: _cameraController->onAction(CameraAction::Down,    pressed); break; // Q
        default: break;
    }
}
void RasterBackend::onScroll(float delta) {
    _cameraController->onScroll(delta);
}
void RasterBackend::onMouseDrag(float dx, float dy) {
    _cameraController->onMouseDrag(dx, dy);
}

/**
 
 scene has list of meshes, meshinstances, and the scenegeometry pool
 pool has the vertex buffer, index buffer, and mesh count
 
 meshes are indexed in the scene containing an index, vertex offset, index offset in the pool buffers
 
 So we need to add meshes by calling void addMeshDirect(Mesh& mesh,
 const std::vector<Vertex>& verts,
 const std::vector<uint32_t>& indices,
 MTL::Device* device); from scene (fills the buffers after we do for all)
 
 
 then bare minimum is to actually render is set the renderencoder and what else?
 */

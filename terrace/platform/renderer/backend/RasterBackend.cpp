//
//  RasterBackend.cpp
//  terrace
//
//  Created by celine on 2026-03-30.
//

#include "RasterBackend.hpp"

RasterBackend::~RasterBackend() {
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
    
    // set camera controller (hardcode for now)
    _cameraController = OrbitController();

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
    
    // camera movement based on input actions
    _currentCameraState = _cameraController.update(_currentCameraState, ctx.dt);
    _updateCameraBuffer();
    // CAMERA with TRIPLE BUFFER

    _frameIndex = (_frameIndex + 1) % _maxBuffers;
    _updateCameraBuffer();

    enc->setVertexBuffer(_cameraBuffers[_frameIndex], 0, 1); // camera
    cmd->addCompletedHandler([this](MTL::CommandBuffer*) {
        dispatch_semaphore_signal(_camBufferSemaphore);
    });
    
    // retrieve pool
    IGeometryPool& pool = _scene->geometryPool();
    
    // make a draw per mesh
    for (Mesh& mesh: _scene->meshes()) {
        // offsets are 0 for now because one buffer per mesh for now
        enc->setVertexBuffer(pool.vertexBufferFor(mesh), 0, 0);
        enc->drawIndexedPrimitives(
           MTL::PrimitiveTypeTriangle,
           mesh.numTriangles * 3,
           MTL::IndexTypeUInt32,
           pool.indexBufferFor(mesh),
           0); // when we do mega buffer it's like this mesh.indexOffset * sizeof(uint32_t))
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
    return;
}
void RasterBackend::onScroll(float delta) {
    _cameraController.onScroll(delta);
}
void RasterBackend::onMouseDrag(float dx, float dy) {
    _cameraController.onMouseDrag(dx, dy);
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

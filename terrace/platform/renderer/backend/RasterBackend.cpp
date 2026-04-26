//
//  RasterBackend.cpp
//  terrace
//
//  Created by celine on 2026-03-30.
//

#include "RasterBackend.hpp"

RasterBackend::~RasterBackend() {
    if (_cameraBuffer) {
        _cameraBuffer->release();
    }
    _cameraBuffer = nullptr;
}

RasterBackend::RasterBackend(MTL::Device* device, Scene* scene): _device(device), _scene(scene) {
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
    std::cout << "rasterizer setup all done \n";
}

void RasterBackend::draw(const FrameContext& ctx) {
    // build camera context and projection matrices
    float aspect = (float)ctx.width / (float)ctx.height;
    
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
    
    simd::float2 viewport = { (float)ctx.width, (float)ctx.height };
    enc->setVertexBytes(&viewport, sizeof(viewport), 1);  // [[buffer(1)]]
    
    SceneGeometryPool& pool = _scene->geometryPool();
    std::cout << "Scene mesh count: " << (_scene->meshes().size()) << std::endl;
    
    // make a draw per mesh
    for (Mesh& mesh: _scene->meshes()) {
        // offsets are 0 for now because one buffer per mesh for now
        enc->setVertexBuffer(pool.vertexBufferFor(mesh), 0, 0);
        enc->setBuffer
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
    
    
}



// Private methods

simd_float4x4 RasterBackend::_buildProjectionMatrix() const {
    float ys = 1.0f / tanf(_fov * 0.5f);
    float xs = ys / _aspect;
    float zs = _far / (_near - _far);
    return simd_float4x4{{
        {xs, 0, 0,        0},
        {0, ys, 0,        0},
        {0, 0,  zs,      -1},
        {0, 0,  zs*_near, 0}
    }}
};
simd_float4x4 RasterBackend::_buildViewMatrix() const {
    simd_float4x4 v = matrix_identity_float4x4;
    return v;
};


void RasterBackend::_updateCameraBuffer() {
    CameraUniformRaster cam;
    cam.view = _buildViewMatrix()
    
    cam.viewProjection = simd_mul(_buildProjectionMatrix(), _buildViewMatrix());

    if (!_cameraBuffer) {
        _cameraBuffer = _device->newBuffer(
            &cam, sizeof(CameraUniforms), MTL::ResourceStorageModeShared);
    } else {
        memcpy(_cameraBuffer->contents(), &cam, sizeof(CameraUniforms));
    }
}

void RasterBackend::onResize(uint32_t width, uint32_t height) {
    _width  = width;
    _height = height;
    _aspect = (float)width / (float)height;
    _updateCameraBuffer();
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

//
//  AccelStructures.cpp
//  wisteria
//
//  Created by celine on 2026-06-30.
//

#include "AccelStructures.hpp"

// One shared definition makes CPU/GPU drift impossible; these pin the ABI size.
static_assert(sizeof(Material)     == 32, "Material layout changed");
static_assert(sizeof(InstanceData) == 80, "InstanceData layout changed");

AccelStructures::~AccelStructures() {
    for (auto* b : _blases) if (b) b->release();
    if (_tlas)               _tlas->release();
    if (_instanceBuffer)     _instanceBuffer->release();
    if (_instanceDataBuffer) _instanceDataBuffer->release();
}

void AccelStructures::build(MTL::Device* device, MTL::CommandQueue* queue, Scene& scene) {
    // descriptor() and NS::Array::array() factories return autoreleased objects;
    // drain them once the builds finish.
    NS::AutoreleasePool* pool = NS::AutoreleasePool::alloc()->init();

    _device = device;
    _queue  = queue;

    IGeometryPool& geomPool = scene.geometryPool();
    MTL::Buffer* megaVB = geomPool.vertexBuffer();
    MTL::Buffer* megaIB = geomPool.indexBuffer();

    _blases.clear();
    _blases.reserve(scene.meshes().size());
    for (const Mesh& mesh : scene.meshes()) {
        if (!megaVB || !megaIB || mesh.indexCount < 3) {
            _blases.push_back(nullptr); // keep _blases index aligned with mesh index
            continue;
        }
        _blases.push_back(_buildBLAS(megaVB, megaIB, mesh));
    }

    _buildTLAS(scene);
    pool->release();
}

MTL::AccelerationStructure* AccelStructures::_buildBLAS(MTL::Buffer* megaVB,
                                                        MTL::Buffer* megaIB,
                                                        const Mesh& mesh)
{
    // Positions are the leading float3 of each Vertex in the shared mega VB.
    // Indices are this mesh's sub-range of the mega IB and are already GLOBAL
    // (rebased into mega-VB space), so no base-vertex offset is needed.
    MTL::AccelerationStructureTriangleGeometryDescriptor* geometryDesc =
        MTL::AccelerationStructureTriangleGeometryDescriptor::descriptor();
    geometryDesc->setVertexBuffer(megaVB);
    geometryDesc->setVertexBufferOffset(0);
    geometryDesc->setVertexFormat(MTL::AttributeFormatFloat3);
    geometryDesc->setVertexStride(sizeof(Vertex));
    geometryDesc->setTriangleCount(mesh.numTriangles);
    geometryDesc->setIndexBuffer(megaIB);
    geometryDesc->setIndexBufferOffset(mesh.indexOffset * sizeof(uint32_t));
    geometryDesc->setIndexType(MTL::IndexTypeUInt32);

    const NS::Object* geometryDescMapped[] = { geometryDesc };
    MTL::PrimitiveAccelerationStructureDescriptor* desc =
        MTL::PrimitiveAccelerationStructureDescriptor::descriptor();
    desc->setGeometryDescriptors(NS::Array::array(geometryDescMapped, 1)); // one mesh per BLAS

    return _buildOne(desc);
}

void AccelStructures::_buildTLAS(Scene& scene) {
    std::vector<MeshInstance>& instances = scene.instances();
    const NS::UInteger n = instances.size();
    if (n == 0) return;

    // One instance descriptor per MeshInstance, in instance order — the shader's
    // instance_id is exactly the index into scene.instances(). Shared storage so
    // we can fill contents() from the CPU.
    _instanceBuffer = _device->newBuffer(
        n * sizeof(MTL::AccelerationStructureInstanceDescriptor),
        MTL::ResourceStorageModeShared);
    _instanceDataBuffer = _device->newBuffer(
        n * sizeof(InstanceData), MTL::ResourceStorageModeShared);

    auto* instDescs = static_cast<MTL::AccelerationStructureInstanceDescriptor*>(_instanceBuffer->contents());
    auto* instData  = static_cast<InstanceData*>(_instanceDataBuffer->contents());
    for (NS::UInteger i = 0; i < n; ++i) {
        const MeshInstance& inst = instances[i];
        instDescs[i].transformationMatrix            = from4x4toPacked4x3(inst.transform);
        instDescs[i].options                         = MTL::AccelerationStructureInstanceOptionDisableTriangleCulling
                                                     | MTL::AccelerationStructureInstanceOptionOpaque;
        instDescs[i].mask                            = 0xFF;
        instDescs[i].intersectionFunctionTableOffset = 0;
        instDescs[i].accelerationStructureIndex      = inst.meshIndex;  // which BLAS in _blases

        // shading payload: instance_id -> this mesh's index range + material + xform
        instData[i].indexOffset = inst.indexOffset;
        instData[i].materialID  = inst.materialID;
        instData[i].transform   = inst.transform;
    }

    MTL::InstanceAccelerationStructureDescriptor* tlasDesc =
        MTL::InstanceAccelerationStructureDescriptor::descriptor();
    tlasDesc->setInstanceCount(n);
    tlasDesc->setInstanceDescriptorBuffer(_instanceBuffer);
    tlasDesc->setInstancedAccelerationStructures(
        NS::Array::array((const NS::Object* const*)_blases.data(), _blases.size()));

    _tlas = _buildOne(tlasDesc);
}

/*
Build any one acceleration structure and block until it's ready.
One-time setup, so the wait is fine. TODO: batch the builds.
  1. size it (accelerationStructureSize + buildScratchBufferSize)
  2. allocate the structure and scratch buffer
  3. encode the build
  4. run + wait (blocking — why we do this at startup)
  5. free scratch
*/
MTL::AccelerationStructure* AccelStructures::_buildOne(MTL::AccelerationStructureDescriptor* desc) {
    MTL::AccelerationStructureSizes sizes = _device->accelerationStructureSizes(desc);
    MTL::AccelerationStructure* as = _device->newAccelerationStructure(sizes.accelerationStructureSize);
    MTL::Buffer* scratch = _device->newBuffer(sizes.buildScratchBufferSize,
                                              MTL::ResourceStorageModePrivate);

    MTL::CommandBuffer* cmd = _queue->commandBuffer();
    auto* enc = cmd->accelerationStructureCommandEncoder();
    enc->buildAccelerationStructure(as, desc, scratch, 0);
    enc->endEncoding();
    cmd->commit();
    cmd->waitUntilCompleted();

    scratch->release();
    return as;
}

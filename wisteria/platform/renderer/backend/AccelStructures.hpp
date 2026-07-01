//
//  AccelStructures.hpp
//  wisteria
//
//  Created by celine on 2026-06-30.
//

#pragma once
#include <Metal/Metal.hpp>
#include <vector>
#include "platform/scene/Scene.hpp"
#include "platform/geometry/GeometryUtils.hpp"

// CPU mirror of the shader-side InstanceData (pathtracer/Types.hpp). Maps a
// hit's instance_id back to its mesh's index range + material. Keep in sync.
struct InstanceData {
    uint32_t indexOffset;
    uint32_t materialID;
    simd_float4x4 transform;   // object -> world; mirrors Types.hpp (16-byte aligned)
};

// Owns the path tracer's Metal BVH: one BLAS per unique mesh (built from the
// mega-buffer index ranges) and one TLAS over all MeshInstances. build() is a
// one-time, blocking setup meant to run after the geometry pool is finalized.
class AccelStructures {
public:
    AccelStructures() = default;
    ~AccelStructures();

    void build(MTL::Device* device, MTL::CommandQueue* queue, Scene& scene);

    MTL::AccelerationStructure* tlas() const { return _tlas; }
    MTL::Buffer* instanceData() const { return _instanceDataBuffer; }
    const std::vector<MTL::AccelerationStructure*>& blases() const { return _blases; }

private:
    MTL::AccelerationStructure* _buildBLAS(MTL::Buffer* megaVB, MTL::Buffer* megaIB, const Mesh& mesh);
    void                        _buildTLAS(Scene& scene);
    MTL::AccelerationStructure* _buildOne(MTL::AccelerationStructureDescriptor* desc);

    MTL::Device*       _device = nullptr;   // cached in build()
    MTL::CommandQueue* _queue  = nullptr;

    std::vector<MTL::AccelerationStructure*> _blases;   // indexed by mesh index
    MTL::AccelerationStructure* _tlas               = nullptr;
    MTL::Buffer*                _instanceBuffer     = nullptr;  // Metal instance descriptors (TLAS build)
    MTL::Buffer*                _instanceDataBuffer = nullptr;  // shading payload the kernel reads
};

//
//  sceneTests.mm
//  wisteria
//
//  Created by celine on 2026-04-27.
//

#import <XCTest/XCTest.h>
#include "platform/scene/Scene.hpp"
#include "MockGeometryPool.hpp"

@interface sceneTests : XCTestCase
@end

@implementation sceneTests

// Instancing: register a mesh once, place it many times. Mirrors the original
// addMeshInstance tests, now on the addMesh/addInstance split (loadObjScene is the
// multi-material file front-end; addMeshFromData is addMesh + one addInstance).

- (void)testInstancingReusesMeshUpload {
    MockGeometryPool pool;
    Scene scene(pool);
    simd::float4x4 I = matrix_identity_float4x4;

    std::vector<Vertex>   verts(3);
    std::vector<uint32_t> idx = {0, 1, 2};

    uint32_t mesh = scene.addMesh(verts, idx, nullptr);   // uploaded once
    scene.addInstance(mesh, I, 0);
    scene.addInstance(mesh, I, 0);

    XCTAssertEqual(scene.numMeshes(), 1);
    XCTAssertEqual(scene.numMeshInstances(), 2);
    XCTAssertEqual(pool._uploadCount, 1);                 // key: one upload, two instances

    const Mesh& m = scene.mesh(0);
    XCTAssertEqual(m.meshInstanceIndexes.size(), 2);
    XCTAssertEqual(m.meshInstanceIndexes[0], 0);
    XCTAssertEqual(m.meshInstanceIndexes[1], 1);
}

- (void)testInstanceIndexesAcrossMeshes {
    MockGeometryPool pool;
    Scene scene(pool);
    simd::float4x4 I = matrix_identity_float4x4;

    std::vector<Vertex>   verts(3);
    std::vector<uint32_t> idx = {0, 1, 2};

    uint32_t meshA = scene.addMesh(verts, idx, nullptr);
    uint32_t meshB = scene.addMesh(verts, idx, nullptr);

    scene.addInstance(meshA, I, 0);
    scene.addInstance(meshB, I, 0);
    scene.addInstance(meshA, I, 0);

    XCTAssertEqual(scene.numMeshes(), 2);
    XCTAssertEqual(scene.numMeshInstances(), 3);
    XCTAssertEqual(pool._uploadCount, 2);                 // meshA + meshB, each uploaded once

    XCTAssertEqual(scene.instance(0).meshIndex, meshA);
    XCTAssertEqual(scene.instance(1).meshIndex, meshB);
    XCTAssertEqual(scene.instance(2).meshIndex, meshA);

    XCTAssertEqual(scene.mesh(meshA).meshInstanceIndexes.size(), 2);
    XCTAssertEqual(scene.mesh(meshB).meshInstanceIndexes.size(), 1);
    XCTAssertEqual(scene.mesh(meshA).meshInstanceIndexes[0], 0);
    XCTAssertEqual(scene.mesh(meshA).meshInstanceIndexes[1], 2);
    XCTAssertEqual(scene.mesh(meshB).meshInstanceIndexes[0], 1);
}

- (void)testAddLightStampsTwoWayLink {
    MockGeometryPool pool;
    Scene scene(pool);
    simd::float4x4 I = matrix_identity_float4x4;

    std::vector<Vertex>   verts(3);
    std::vector<uint32_t> idx = {0, 1, 2};
    uint32_t inst = scene.addMeshFromData(verts, idx, I, 0, nullptr);

    XCTAssertEqual(scene.instance(inst).lightID, -1);               // not emissive yet

    Light L{};
    L.type     = LIGHT_AREA;
    L.radiance = {5.0f, 5.0f, 5.0f};
    L.twoSided = 0;
    uint32_t lightID = scene.addLight(L, inst);

    XCTAssertEqual(scene.numLights(), 1u);
    XCTAssertEqual(scene.instance(inst).lightID, (int32_t)lightID); // instance -> light
    XCTAssertEqual(scene.lights()[lightID].instanceID, inst);       // light -> instance
}

@end

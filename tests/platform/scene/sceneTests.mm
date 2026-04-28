//
//  sceneTests.mm
//  terrace
//
//  Created by celine on 2026-04-27.
//

#import <XCTest/XCTest.h>
#include "platform/scene/Scene.hpp"
#include "MockGeometryPool.hpp"

@interface sceneTests : XCTestCase
@end

@implementation sceneTests

- (void)testMeshInstancesDontReuploadMesh {
    MockGeometryPool pool;
    Scene scene(pool);

    std::string path = "test_mesh.obj";
    scene.addMeshInstance(path, nullptr); // device is nullptr, mock ignores it
    scene.addMeshInstance(path, nullptr);

    XCTAssertEqual(scene.numMeshes(), 1);
    XCTAssertEqual(scene.numMeshInstances(), 2);
    XCTAssertEqual(pool._uploadCount, 1); // key assertion: mesh only uploaded once

    const Mesh& mesh = scene.mesh(0);
    XCTAssertEqual(mesh.meshInstanceIndexes.size(), 2);
    XCTAssertEqual(mesh.meshInstanceIndexes[0], 0);
    XCTAssertEqual(mesh.meshInstanceIndexes[1], 1);
}

- (void)testMeshInstanceIndexesAreCorrect {
    MockGeometryPool pool;
    Scene scene(pool);

    std::string meshA = "a.obj";
    std::string meshB = "b.obj";

    scene.addMeshInstance(meshA, nullptr);
    scene.addMeshInstance(meshB, nullptr);
    scene.addMeshInstance(meshA, nullptr);

    XCTAssertEqual(scene.numMeshes(), 2);
    XCTAssertEqual(scene.numMeshInstances(), 3);
    XCTAssertEqual(pool._uploadCount, 2); // a.obj and b.obj, a.obj not re-uploaded

    XCTAssertEqual(scene.instance(0).meshIndex, 0);
    XCTAssertEqual(scene.instance(1).meshIndex, 1);
    XCTAssertEqual(scene.instance(2).meshIndex, 0);

    const Mesh& mesh0 = scene.mesh(0);
    const Mesh& mesh1 = scene.mesh(1);

    XCTAssertEqual(mesh0.meshInstanceIndexes.size(), 2);
    XCTAssertEqual(mesh1.meshInstanceIndexes.size(), 1);

    XCTAssertEqual(mesh0.meshInstanceIndexes[0], 0);
    XCTAssertEqual(mesh0.meshInstanceIndexes[1], 2);
    XCTAssertEqual(mesh1.meshInstanceIndexes[0], 1);
}

@end

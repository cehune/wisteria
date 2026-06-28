//
//  cameraTests.mm
//  wisteria
//
//  Created by celine on 2026-04-26.
//
#import <XCTest/XCTest.h>
#include "simd/simd.h"
#include "engine/scene/Camera/Camera.hpp"
#include "engine/scene/Camera/OrbitController.hpp"

static const float EPS = 1e-4f;

static bool vec3Equal(simd_float3 a, simd_float3 b, float eps = EPS) {
    return fabs(a.x-b.x) < eps && fabs(a.y-b.y) < eps && fabs(a.z-b.z) < eps;
}

static bool mat4Equal(simd_float4x4 A, simd_float4x4 B, float eps = EPS) {
    for (int c = 0; c < 4; ++c)
        for (int r = 0; r < 4; ++r)
            if (fabs(A.columns[c][r] - B.columns[c][r]) > eps) return false;
    return true;
}

@interface cameraTests : XCTestCase
@end

@implementation cameraTests

- (void)testViewMatrixTranslation {
    CameraState state;
    state.position = simd_make_float3(0, 0, 5);
    Camera cam;

    simd_float4x4 view = cam.viewMatrix(state);
    simd_float4 viewSpace = simd_mul(view, simd_make_float4(0, 0, 0, 1));

    XCTAssertEqualWithAccuracy(viewSpace.z, -5.0f, EPS);
}

- (void)testViewMatrixIdentityAtOrigin {
    CameraState state; // defaults: position={0,0,0}, orientation=identity
    Camera cam;
    XCTAssert(mat4Equal(cam.viewMatrix(state), matrix_identity_float4x4));
}

- (void)testForwardBasisVector {
    CameraState state;
    // forward = simd_act(identity_quat, {0,0,-1}) = {0,0,-1}
    simd_float3 fwd = simd_act(state.orientation, simd_make_float3(0, 0, -1));
    XCTAssert(vec3Equal(fwd, simd_make_float3(0, 0, -1)));
}

- (void)testRightBasisVector {
    CameraState state;
    simd_float3 r = simd_act(state.orientation, simd_make_float3(1, 0, 0));
    XCTAssert(vec3Equal(r, simd_make_float3(1, 0, 0)));
}

- (void)testUpBasisVector {
    CameraState state;
    simd_float3 u = simd_act(state.orientation, simd_make_float3(0, 1, 0));
    XCTAssert(vec3Equal(u, simd_make_float3(0, 1, 0)));
}

- (void)testBasisOrthonormalityAfterRotation {
    // simulate what OrbitController produces after some yaw/pitch
    OrbitController controller;
    CameraState state;
    // apply some drag to get a non-trivial orientation
    controller.onMouseDrag(0.4f, 0.2f);
    state = controller.update(state, 0.0f);

    simd_float3 f = simd_act(state.orientation, simd_make_float3(0, 0, -1));
    simd_float3 r = simd_act(state.orientation, simd_make_float3(1, 0, 0));
    simd_float3 u = simd_act(state.orientation, simd_make_float3(0, 1, 0));

    XCTAssertEqualWithAccuracy(simd_dot(f, r), 0.0f, EPS);
    XCTAssertEqualWithAccuracy(simd_dot(f, u), 0.0f, EPS);
    XCTAssertEqualWithAccuracy(simd_dot(r, u), 0.0f, EPS);
    XCTAssertEqualWithAccuracy(simd_length(f), 1.0f, EPS);
    XCTAssertEqualWithAccuracy(simd_length(r), 1.0f, EPS);
    XCTAssertEqualWithAccuracy(simd_length(u), 1.0f, EPS);
}

- (void)testProjectionNearPlaneNDC {
    CameraState state;
    Camera cam;
    simd_float4x4 P = cam.projectionMatrix(state, 1.0f);
    simd_float4 nearPoint = simd_make_float4(0, 0, -state.near, 1);
    simd_float4 clip = simd_mul(P, nearPoint);
    XCTAssertEqualWithAccuracy(clip.z / clip.w, 0.0f, EPS);
}

- (void)testProjectionFarPlaneNDC {
    CameraState state;
    Camera cam;
    simd_float4x4 P = cam.projectionMatrix(state, 1.0f);
    simd_float4 farPoint = simd_make_float4(0, 0, -state.far, 1);
    simd_float4 clip = simd_mul(P, farPoint);
    XCTAssertEqualWithAccuracy(clip.z / clip.w, 1.0f, EPS);
}

@end

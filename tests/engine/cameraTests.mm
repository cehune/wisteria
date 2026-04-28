//
//  cameraTests.mm
//  terrace
//
//  Created by celine on 2026-04-26.
//
#import <XCTest/XCTest.h>
#include "simd/simd.h"
#include "engine/scene/Camera/Camera.hpp"

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
    // camera at z=5 looking toward -z, origin should appear at z=-5 in view space
    Camera cam;
    cam.position = simd_make_float3(0, 0, 5);

    simd_float4x4 view = cam.viewMatrix();
    simd_float4 worldOrigin = simd_make_float4(0, 0, 0, 1);
    simd_float4 viewSpace = simd_mul(view, worldOrigin);

    XCTAssertEqualWithAccuracy(viewSpace.x,  0.0f, EPS);
    XCTAssertEqualWithAccuracy(viewSpace.y,  0.0f, EPS);
    XCTAssertEqualWithAccuracy(viewSpace.z, -5.0f, EPS);
    XCTAssertEqualWithAccuracy(viewSpace.w,  1.0f, EPS);
}

- (void)testViewMatrixIdentityAtOrigin {
    // camera at origin with no rotation, view matrix should be identity
    Camera cam;
    XCTAssert(mat4Equal(cam.viewMatrix(), matrix_identity_float4x4));
}

- (void)testForwardBasisVector {
    // default orientation should point toward -z
    Camera cam;
    simd_float3 fwd = cam.forward();
    XCTAssert(vec3Equal(fwd, simd_make_float3(0, 0, -1)));
}

- (void)testRightBasisVector {
    Camera cam;
    simd_float3 r = cam.right();
    XCTAssert(vec3Equal(r, simd_make_float3(1, 0, 0)));
}

- (void)testUpBasisVector {
    Camera cam;
    simd_float3 u = cam.up();
    XCTAssert(vec3Equal(u, simd_make_float3(0, 1, 0)));
}

- (void)testBasisOrthonormalityAfterRotation {
    // after a rotation, forward/right/up should remain mutually orthonormal
    Camera cam;
    cam.rotate(0.4f, 0.2f);

    simd_float3 f = cam.forward();
    simd_float3 r = cam.right();
    simd_float3 u = cam.up();

    // dot products between distinct axes should be ~0
    XCTAssertEqualWithAccuracy(simd_dot(f, r), 0.0f, EPS);
    XCTAssertEqualWithAccuracy(simd_dot(f, u), 0.0f, EPS);
    XCTAssertEqualWithAccuracy(simd_dot(r, u), 0.0f, EPS);

    // each axis should be unit length
    XCTAssertEqualWithAccuracy(simd_length(f), 1.0f, EPS);
    XCTAssertEqualWithAccuracy(simd_length(r), 1.0f, EPS);
    XCTAssertEqualWithAccuracy(simd_length(u), 1.0f, EPS);
}

- (void)testProjectionNearPlaneNDC {
    // a point on the near plane should map to NDC z = 0 in Metal
    Camera cam;
    float aspect = 1.0f;
    simd_float4x4 P = cam.projectionMatrix(aspect);

    simd_float4 nearPoint = simd_make_float4(0, 0, -cam.getNear(), 1);
    simd_float4 clip = simd_mul(P, nearPoint);
    float ndcZ = clip.z / clip.w;
    XCTAssertEqualWithAccuracy(ndcZ, 0.0f, EPS);
}

- (void)testProjectionFarPlaneNDC {
    // a point on the far plane should map to NDC z = 1 in Metal
    Camera cam;
    float aspect = 1.0f;
    simd_float4x4 P = cam.projectionMatrix(aspect);

    simd_float4 farPoint = simd_make_float4(0, 0, -cam.getFar(), 1);
    simd_float4 clip = simd_mul(P, farPoint);
    float ndcZ = clip.z / clip.w;
    XCTAssertEqualWithAccuracy(ndcZ, 1.0f, EPS);
}

@end

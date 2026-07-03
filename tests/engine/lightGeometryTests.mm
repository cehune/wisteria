//
//  lightGeometryTests.mm
//  wisteria
//
//  Host unit tests for the shared area-light -> solid-angle pdf (area_pdf_direction).
//  Each test isolates one factor of the density so a formula regression points
//  straight at the broken term. No GPU: LightGeometry.h compiles under host C++/simd.
//

#import <XCTest/XCTest.h>
#include <simd/simd.h>
#include "engine/gpu/LightGeometry.h"

static inline simd_float3 f3(float x, float y, float z) { return simd_make_float3(x, y, z); }

@interface lightGeometryTests : XCTestCase
@end

@implementation lightGeometryTests

// Reference: right triangle in the z=0 plane, area 0.5, unit normal +z.
// refP sits 2 units up the normal over an interior point looking straight down:
//   pdf = dist^2 / (numTri * area * cosL) = 4 / (1 * 0.5 * 1) = 8.
- (void)testClosedForm {
    simd_float3 a = f3(0,0,0), b = f3(1,0,0), c = f3(0,1,0);
    float pdf = area_pdf_direction(a, b, c, f3(0.25f, 0.25f, 2.0f), f3(0.25f, 0.25f, 0.0f), 1.0f);
    XCTAssertEqualWithAccuracy(pdf, 8.0f, 1e-3f);
}

// Inverse-square law: doubling the distance along the normal quadruples the pdf.
- (void)testDistanceSquaredLaw {
    simd_float3 a = f3(0,0,0), b = f3(1,0,0), c = f3(0,1,0);
    simd_float3 lp = f3(0.25f, 0.25f, 0.0f);
    float nearP = area_pdf_direction(a, b, c, f3(0.25f, 0.25f, 2.0f), lp, 1.0f);
    float farP  = area_pdf_direction(a, b, c, f3(0.25f, 0.25f, 4.0f), lp, 1.0f);
    XCTAssertEqualWithAccuracy(farP, 4.0f * nearP, 1e-2f);
}

// Cosine (foreshortening): same distance, but cosL = 0.5 at the light -> pdf x2.
// refP = lp - 2*wi with wi = (sqrt3/2, 0, -1/2): |d| = 2, |wi.n| = 0.5.
- (void)testCosineFactor {
    simd_float3 a = f3(0,0,0), b = f3(1,0,0), c = f3(0,1,0);
    simd_float3 lp = f3(0.25f, 0.25f, 0.0f);
    simd_float3 refP = f3(-1.4820508f, 0.25f, 1.0f);
    float pdf = area_pdf_direction(a, b, c, refP, lp, 1.0f);
    XCTAssertEqualWithAccuracy(pdf, 16.0f, 1e-2f);
}

// Area factor: 2x the edges -> 4x the area -> 1/4 the pdf (8 -> 2).
- (void)testAreaFactor {
    simd_float3 a = f3(0,0,0), b = f3(2,0,0), c = f3(0,2,0);
    float pdf = area_pdf_direction(a, b, c, f3(0.5f, 0.5f, 2.0f), f3(0.5f, 0.5f, 0.0f), 1.0f);
    XCTAssertEqualWithAccuracy(pdf, 2.0f, 1e-3f);
}

// Uniform triangle selection: numTri = 4 -> 1/4 the per-point pdf (8 -> 2).
- (void)testTriangleCountFactor {
    simd_float3 a = f3(0,0,0), b = f3(1,0,0), c = f3(0,1,0);
    simd_float3 refP = f3(0.25f, 0.25f, 2.0f), lp = f3(0.25f, 0.25f, 0.0f);
    float one  = area_pdf_direction(a, b, c, refP, lp, 1.0f);
    float four = area_pdf_direction(a, b, c, refP, lp, 4.0f);
    XCTAssertEqualWithAccuracy(four, one / 4.0f, 1e-3f);
    XCTAssertEqualWithAccuracy(four, 2.0f, 1e-3f);
}

// Degenerate (collinear) triangle has zero area -> pdf 0, no divide-by-zero.
- (void)testDegenerateTriangleIsZero {
    simd_float3 a = f3(0,0,0), b = f3(1,0,0), c = f3(2,0,0);
    float pdf = area_pdf_direction(a, b, c, f3(0,0,1), f3(1,0,0), 1.0f);
    XCTAssertEqual(pdf, 0.0f);
}

@end

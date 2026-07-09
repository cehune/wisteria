//
//  conductorSmoothTests.mm
//  wisteria
//
//  Created by celine on 2026-07-09.
//

#import <XCTest/XCTest.h>
#include <simd/simd.h>
#include "engine/shading/bxdf/Conductor.h"
#include "../TestUtils.h"

static const simd_float3 kWo = simd_normalize(f3(0.4f, 0.0f, 0.9f));

@interface conductorSmoothTests : XCTestCase
@end

@implementation conductorSmoothTests

- (void)testIsDeltaIsSet {
    BSDFSample bs = conductor_smooth_sample(f3(1, 1, 1), f3(0, 0, 0), f3(0, 0, 0), false, kWo);
    XCTAssertTrue(bs.isDelta);
}

// Reflection is the exact mirror about the shading normal -- no VNDF spread.
- (void)testReflectionIsExactMirror {
    BSDFSample bs = conductor_smooth_sample(f3(1, 1, 1), f3(0, 0, 0), f3(0, 0, 0), false, kWo);
    simd_float3 mirror = f3(-kWo.x, -kWo.y, kWo.z);
    XCTAssertEqualWithAccuracy(bs.wi.x, mirror.x, 1e-6f);
    XCTAssertEqualWithAccuracy(bs.wi.y, mirror.y, 1e-6f);
    XCTAssertEqualWithAccuracy(bs.wi.z, mirror.z, 1e-6f);
}

// Reflection is the only outcome -- no branch probability, unlike dielectric.
- (void)testPdfIsAlwaysOne {
    BSDFSample bs = conductor_smooth_sample(f3(0.9f, 0.6f, 0.2f), f3(0, 0, 0), f3(0, 0, 0), false, kWo);
    XCTAssertEqualWithAccuracy(bs.pdf, 1.0f, 1e-6f);
}

// Below the geometric surface: single-sided, same convention as the rough conductor path.
- (void)testBelowSurfaceReturnsZeroPdf {
    simd_float3 woBelow = f3(0.4f, 0.0f, -0.9f);
    BSDFSample  bs      = conductor_smooth_sample(f3(1, 1, 1), f3(0, 0, 0), f3(0, 0, 0), false, woBelow);
    XCTAssertEqual(bs.pdf, 0.0f);
}

// Schlick fallback (hasComplexIOR=false): throughput = f*|wi.z|/pdf must equal F0 exactly
// at normal incidence, where Schlick's curve always reduces to F0 regardless of formula.
- (void)testThroughputMatchesSchlickFresnel {
    simd_float3 wo = f3(0.0f, 0.0f, 1.0f);   // normal incidence
    simd_float3 F0 = f3(0.9f, 0.6f, 0.2f);
    BSDFSample  bs = conductor_smooth_sample(F0, f3(0, 0, 0), f3(0, 0, 0), false, wo);

    float throughput = bs.f.x * fabsf(bs.wi.z) / bs.pdf;
    XCTAssertEqualWithAccuracy(throughput, F0.x, 1e-5f);
}

// Exact path (hasComplexIOR=true): throughput must match fresnel_conductor_exact directly
// -- cross-check against the already-validated Fresnel function, not a re-derivation.
- (void)testThroughputMatchesExactFresnel {
    simd_float3 eta = f3(0.2f, 0.92f, 1.10f);   // roughly copper
    simd_float3 k   = f3(3.91f, 2.45f, 2.14f);
    BSDFSample  bs  = conductor_smooth_sample(f3(1, 1, 1), eta, k, true, kWo);

    simd_float3 expectedF = fresnel_conductor_exact(kWo.z, 1.0f, eta, k);
    float       throughput = bs.f.x * fabsf(bs.wi.z) / bs.pdf;
    XCTAssertEqualWithAccuracy(throughput, expectedF.x, 1e-5f);
}

@end

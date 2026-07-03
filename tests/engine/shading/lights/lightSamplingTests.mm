//
//  lightSamplingTests.mm
//  wisteria
//
//  Host unit tests for the pure light samplers (engine/shading/lights/LightSampling.h).
//  These run off-GPU because the samplers take explicit geometry — the device fetch
//  lives in platform/Light.h. Covers sample<->pdf reciprocity, emit-side facing, the
//  delta lights, and a Monte-Carlo check that the sampling density equals the analytic
//  solid angle (the test that would actually catch a measure/Jacobian bug).
//

#import <XCTest/XCTest.h>
#include <simd/simd.h>
#include <random>
#include <cmath>
#include "engine/shading/lights/LightSampling.h"

static inline simd_float3 f3(float x, float y, float z) { return simd_make_float3(x, y, z); }

// Van Oosterom–Strackee: solid angle subtended by triangle (A,B,C) at refP.
static double triSolidAngle(simd_float3 refP, simd_float3 A, simd_float3 B, simd_float3 C) {
    simd_float3 a = simd_normalize(A - refP);
    simd_float3 b = simd_normalize(B - refP);
    simd_float3 c = simd_normalize(C - refP);
    double num = fabs((double)simd_dot(a, simd_cross(b, c)));
    double den = 1.0 + (double)simd_dot(a, b) + (double)simd_dot(b, c) + (double)simd_dot(a, c);
    return 2.0 * atan2(num, den);
}

@interface lightSamplingTests : XCTestCase
@end

@implementation lightSamplingTests

// The sampled point reconstructs from wi*dist, and its pdf matches area_pdf_direction.
- (void)testAreaReciprocity {
    simd_float3 A = f3(0,0,0), B = f3(1,0,0), C = f3(0,1,0), refP = f3(0.2f, 0.2f, 1.0f);
    LightSample s = light_sample_area(A, B, C, f3(1,1,1), true, refP, simd_make_float2(0.3f, 0.6f), 1u);
    simd_float3 lp = refP + s.wi * s.dist;
    float p = area_pdf_direction(A, B, C, refP, lp, 1.0f);
    XCTAssertEqualWithAccuracy(p, s.pdf, 1e-4f);
    XCTAssertEqualWithAccuracy(simd_length(s.wi), 1.0f, 1e-5f);
}

// One-sided light emits only along +geometric-normal; two-sided emits both ways.
- (void)testAreaFacing {
    simd_float3 A = f3(0,0,0), B = f3(1,0,0), C = f3(0,1,0);   // geometric normal +z
    simd_float2 bary = simd_make_float2(0.3f, 0.3f);
    LightSample front = light_sample_area(A, B, C, f3(5,5,5), false, f3(0.25f,0.25f, 1.0f), bary, 1u);
    LightSample back  = light_sample_area(A, B, C, f3(5,5,5), false, f3(0.25f,0.25f,-1.0f), bary, 1u);
    LightSample two   = light_sample_area(A, B, C, f3(5,5,5), true,  f3(0.25f,0.25f,-1.0f), bary, 1u);
    XCTAssertGreaterThan(front.Li.x, 0.0f);
    XCTAssertEqual(back.Li.x, 0.0f);
    XCTAssertGreaterThan(two.Li.x, 0.0f);
}

- (void)testPointLight {
    LightSample s = light_sample_point(f3(0,0,3), f3(7,7,7), f3(0,0,0));
    XCTAssertEqualWithAccuracy(s.dist, 3.0f, 1e-5f);
    XCTAssertEqualWithAccuracy(s.wi.z, 1.0f, 1e-5f);
    XCTAssertEqual(s.pdf, 1.0f);
    XCTAssertEqualWithAccuracy(s.Li.x, 7.0f, 1e-6f);
}

- (void)testDirectionalLight {
    LightSample s = light_sample_directional(f3(0,0,-1), f3(2,2,2));
    XCTAssertEqualWithAccuracy(s.wi.z, 1.0f, 1e-5f);   // wi = -dir
    XCTAssertEqual(s.pdf, 1.0f);
    XCTAssertTrue(std::isinf(s.dist));
    XCTAssertEqualWithAccuracy(s.Li.x, 2.0f, 1e-6f);
}

// E[1/pdf] over uniform-area samples == solid angle subtended: the sampling density
// matches the reported solid-angle pdf, and both are geometrically correct.
- (void)testAreaDensityMatchesSolidAngle {
    simd_float3 A = f3(0,0,0), B = f3(1,0,0), C = f3(0,1,0), refP = f3(0.2f, 0.2f, 1.0f);
    double omega = triSolidAngle(refP, A, B, C);

    std::mt19937 rng(1234567u);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    const int N = 200000;
    double acc = 0.0;
    for (int i = 0; i < N; ++i) {
        LightSample s = light_sample_area(A, B, C, f3(1,1,1), true, refP,
                                          simd_make_float2(U(rng), U(rng)), 1u);
        acc += 1.0 / (double)s.pdf;
    }
    double est = acc / N;
    XCTAssertEqualWithAccuracy(est, omega, omega * 0.03);   // within 3%
}

@end

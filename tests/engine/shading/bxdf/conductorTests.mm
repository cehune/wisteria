//
//  conductorTests.mm
//  wisteria
// 
//  Created by celine on 2026-07-08.
//

#import <XCTest/XCTest.h>
#include <simd/simd.h>
#include <random>
#include "engine/shading/bxdf/Conductor.h"
#include "BxdfConformance.h"
#include "../TestUtils.h"

using namespace bxdftest;

static const simd_float3 kWo = simd_normalize(f3(0.4f, 0.0f, 0.9f));

@interface conductorTests : XCTestCase
@end

@implementation conductorTests

// Schlick: F(cos=1) == F0 (facing), F(cos=0) == 1 (grazing = total reflection).
- (void)testFresnelEndpoints {
    simd_float3 F0 = f3(0.04f, 0.5f, 0.95f);
    simd_float3 facing  = fresnel_conductor(F0, 1.0f);
    simd_float3 grazing = fresnel_conductor(F0, 0.0f);
    XCTAssertEqualWithAccuracy(facing.x,  F0.x, 1e-6f);
    XCTAssertEqualWithAccuracy(facing.z,  F0.z, 1e-6f);
    XCTAssertEqualWithAccuracy(grazing.x, 1.0f, 1e-6f);
    XCTAssertEqualWithAccuracy(grazing.z, 1.0f, 1e-6f);
}

// A proper NDF integrates to 1: ∫ D(m) cos(theta_m) dω_m = 1.
- (void)testGgxNdfNormalization {
    const float a = 0.5f;
    std::mt19937 rng(99u);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    const int N = 200000;
    double acc = 0.0;
    for (int i = 0; i < N; ++i) {
        simd_float3 m = ::uniformHemisphere(U(rng), U(rng));
        acc += (double)ggx_ndf(m, a) * (double)m.z;
    }
    double integral = acc * (2.0 * M_PI) / N;   // divide out the uniform pdf (1/2pi)
    XCTAssertEqualWithAccuracy(integral, 1.0, 0.03);
}

// Near-zero roughness: the sampled direction is the mirror reflection about +z.
- (void)testNearMirrorDirection {
    BSDFSample bs = conductor_sample(f3(1,1,1), 1e-4f, kWo, f2(0.5f, 0.5f));
    simd_float3 mirror = f3(-kWo.x, -kWo.y, kWo.z);
    XCTAssertGreaterThan(bs.pdf, 0.0f);
    XCTAssertGreaterThan((double)simd_dot(bs.wi, mirror), 0.999);
}

// Energy: importance-sampled directional albedo must agree with an independent
// brute-force estimator and never exceed 1 (no gain), across a spread of roughness.
- (void)testEnergyConservationAcrossRoughness {
    const float alphas[] = { 0.05f, 0.2f, 0.5f };
    for (float a : alphas) {
        SampleFn sample = [a](float3 wo, float2 u) { return conductor_sample(f3(1,1,1), a, wo, u); };
        EvalFn   eval   = [a](float3 wo, float3 wi) { return conductor_eval(f3(1,1,1), a, wo, wi); };

        float3 rhoImportance = directionalAlbedoImportance(sample, kWo, 10u);
        float3 rhoBrute      = directionalAlbedoBruteForce(eval, kWo, 11u);

        XCTAssertLessThanOrEqual(rhoImportance.x, 1.01f);
        XCTAssertEqualWithAccuracy(rhoImportance.x, rhoBrute.x, 0.03f);  // two independent estimators agree
        if (a == 0.05f) XCTAssertGreaterThan(rhoImportance.x, 0.9f);     // little loss when smooth
    }
}

- (void)testPdfIntegratesToOne {
    const float a = 0.3f;
    PdfFn pdf = [a](float3 wo, float3 wi) { return conductor_pdf(a, wo, wi); };
    double integral = pdfIntegral(pdf, kWo, 12u);
    XCTAssertEqualWithAccuracy(integral, 1.0, 0.05);
}

- (void)testReciprocity {
    const float a = 0.3f;
    simd_float3 F0 = f3(0.7f, 0.5f, 0.3f);
    EvalFn eval = [a, F0](float3 wo, float3 wi) { return conductor_eval(F0, a, wo, wi); };
    float err = maxReciprocityError(eval, 13u);
    XCTAssertLessThan(err, 1e-4f);
}

- (void)testSamplerMatchesOwnPdfChiSquare {
    const float a = 0.3f;
    SampleFn sample = [a](float3 wo, float2 u) { return conductor_sample(f3(1,1,1), a, wo, u); };
    PdfFn    pdf    = [a](float3 wo, float3 wi) { return conductor_pdf(a, wo, wi); };
    double reducedChi2 = reducedChiSquare(sample, pdf, kWo, 14u);
    XCTAssertLessThan(reducedChi2, 3.0);
}

@end

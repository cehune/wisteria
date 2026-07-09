//
//  lambertianTests.mm
//  wisteria
//
//  Created by celine on 2026-07-08.
//

#import <XCTest/XCTest.h>
#include <simd/simd.h>
#include "engine/shading/bxdf/Lambertian.h"
#include "BxdfConformance.h"
#include "../TestUtils.h"

using namespace bxdftest;

static const simd_float3 kWo = simd_normalize(f3(0.4f, 0.0f, 0.9f));

@interface lambertianTests : XCTestCase
@end

@implementation lambertianTests

- (void)testEvalIsAlbedoOverPiInHemisphere {
    simd_float3 albedo = f3(0.6f, 0.3f, 0.9f);
    simd_float3 wi = f3(0.0f, 0.0f, 1.0f);
    simd_float3 f = lambertian_eval(albedo, kWo, wi);
    XCTAssertEqualWithAccuracy(f.x, albedo.x * (float)M_1_PI, 1e-6f);
}

- (void)testEvalIsZeroAcrossHemisphere {
    simd_float3 albedo = f3(1.0f, 1.0f, 1.0f);
    simd_float3 wi = f3(0.0f, 0.0f, -1.0f);   // opposite hemisphere from kWo
    simd_float3 f = lambertian_eval(albedo, kWo, wi);
    XCTAssertEqual(f.x, 0.0f);
}

// Two independent estimators of rho(wo) — one importance-sampled through
// lambertian_sample, one brute-force through lambertian_eval alone — must agree with
// each other and with the analytic answer (albedo, since f*cos integrates exactly).
- (void)testEnergyConservationImportanceVsBruteForce {
    simd_float3 albedo = f3(0.7f, 0.4f, 0.9f);
    SampleFn sample = [albedo](float3 wo, float2 u) { return lambertian_sample(albedo, wo, u); };
    EvalFn   eval   = [albedo](float3 wo, float3 wi) { return lambertian_eval(albedo, wo, wi); };

    float3 rhoImportance = directionalAlbedoImportance(sample, kWo, 1u);
    float3 rhoBrute      = directionalAlbedoBruteForce(eval, kWo, 2u);

    XCTAssertEqualWithAccuracy(rhoImportance.x, albedo.x, 0.01f);
    XCTAssertEqualWithAccuracy(rhoBrute.x,      albedo.x, 0.02f);
}

- (void)testPdfIntegratesToOne {
    PdfFn pdf = [](float3 wo, float3 wi) { return lambertian_pdf(wo, wi); };
    double integral = pdfIntegral(pdf, kWo, 3u);
    XCTAssertEqualWithAccuracy(integral, 1.0, 0.02);
}

- (void)testReciprocity {
    simd_float3 albedo = f3(0.5f, 0.5f, 0.5f);
    EvalFn eval = [albedo](float3 wo, float3 wi) { return lambertian_eval(albedo, wo, wi); };
    float err = maxReciprocityError(eval, 4u);
    XCTAssertLessThan(err, 1e-6f);
}

- (void)testSamplerMatchesOwnPdfChiSquare {
    simd_float3 albedo = f3(1.0f, 1.0f, 1.0f);
    SampleFn sample = [albedo](float3 wo, float2 u) { return lambertian_sample(albedo, wo, u); };
    PdfFn    pdf    = [](float3 wo, float3 wi) { return lambertian_pdf(wo, wi); };
    double reducedChi2 = reducedChiSquare(sample, pdf, kWo, 5u);
    XCTAssertLessThan(reducedChi2, 3.0);   // heuristic threshold, not a formal p-value test
}

@end

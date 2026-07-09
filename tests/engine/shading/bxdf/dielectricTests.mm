//
//  dielectricTests.mm
//  wisteria
//
//  Created by celine on 2026-07-08.
//

#import <XCTest/XCTest.h>
#include <simd/simd.h>
#include <random>
#include "engine/shading/bxdf/Dielectric.h"
#include "BxdfConformance.h"
#include "../TestUtils.h"

using namespace bxdftest;

static const simd_float3 kWo  = simd_normalize(f3(0.4f, 0.0f, 0.9f));
static const float       kEta = 1.5f; // typical glass, relative to air

@interface dielectricTests : XCTestCase
@end

@implementation dielectricTests

// cosThetaT is smae as cosThetaI
- (void)testFresnelNormalIncidence {
    FresnelResult fr = fresnel_dielectric(1.0f, 1.0f, kEta);
    float expectedF0 = ((kEta - 1.0f) / (kEta + 1.0f)) * ((kEta - 1.0f) / (kEta + 1.0f));
    XCTAssertEqualWithAccuracy(fr.F, expectedF0, 1e-5f);
    XCTAssertEqualWithAccuracy(fr.cosThetaT, 1.0f, 1e-5f);
}

// if graing, F should go to 1
- (void)testFresnelGrazingIncidence {
    FresnelResult fr = fresnel_dielectric(0.0f, 1.0f, kEta);
    XCTAssertEqualWithAccuracy(fr.F, 1.0f, 1e-5f);
}

// transmission must be impossible past the critical angle
- (void)testFresnelTotalInternalReflection {
    // critical angle for eta=1.5 exiting into air is around 
    // theta_c ~= 41.8deg
    // cosThetaI = 0.5 is past it.
    FresnelResult fr = fresnel_dielectric(0.5f, kEta, 1.0f); // etaI=glass, etaT=air
    XCTAssertEqualWithAccuracy(fr.F, 1.0f, 1e-6f);
    XCTAssertEqualWithAccuracy(fr.cosThetaT, 0.0f, 1e-6f);
}

// if etaI and etaT are the same, theres no interface at all, zero reflectance at every angle.
- (void)testFresnelMatchedMediaIsZero {
    FresnelResult fr = fresnel_dielectric(0.3f, 1.5f, 1.5f);
    XCTAssertEqualWithAccuracy(fr.F, 0.0f, 1e-5f);
}

// refraction shouldn't bend at a complete normal to the surface
- (void)testNormalIncidenceTransmissionIsStraightThrough {
    simd_float3 wo = f3(0.0f, 0.0f, 1.0f);
    BSDFSample  bs = dielectric_sample(f3(1, 1, 1), 1e-4f, kEta, wo, f2(0.5f, 0.5f), 0.99f);
    XCTAssertGreaterThan(bs.pdf, 0.0f);
    XCTAssertGreaterThan((double)simd_dot(bs.wi, f3(0.0f, 0.0f, -1.0f)), 0.999);
}

// should correctly mark a reflection as a reflection for a mirrored ray
// on the same hemispher
- (void)testNearMirrorReflectionDirection {
    BSDFSample  bs     = dielectric_sample(f3(1, 1, 1), 1e-4f, kEta, kWo, f2(0.5f, 0.5f), 0.0f);
    simd_float3 mirror = f3(-kWo.x, -kWo.y, kWo.z);
    XCTAssertGreaterThan(bs.pdf, 0.0f);
    XCTAssertGreaterThan((double)simd_dot(bs.wi, mirror), 0.999);
}

// mportance-sampled total (reflection + transmission) should agree with a brute
// force estimator. 
- (void)testEnergyConservationAcrossRoughness {
    const float alphas[] = { 0.05f, 0.2f, 0.5f };
    for (int i = 0; i < 3; ++i) {
        float a = alphas[i];
        // generate a random directioned sample
        SampleFn sample = [a, rng = std::mt19937(2000u + (uint32_t)i),
                            dist = std::uniform_real_distribution<float>(0.0f, 1.0f)]
                           (float3 wo, float2 u) mutable {
            return dielectric_sample(f3(1, 1, 1), a, kEta, wo, u, dist(rng));
        };
        EvalFn eval = [a](float3 wo, float3 wi) { return dielectric_eval(f3(1, 1, 1), a, kEta, wo, wi); };

        // check on the individual albedos
        float3 rhoImportance = directionalAlbedoImportance(sample, kWo, 20u + i);
        float3 rhoBrute      = directionalAlbedoBruteForceFullSphere(eval, kWo, 30u + i);

        // Upper bound only: for this ENTERING configuration (etap = eta > 1), the radiance-
        // compression identity F + (1-F)/etap^2 is algebraically < 1 here (1/etap^2 < 1),
        // so this is a real physical ceiling -- not a loose "no gain" sanity check.
        XCTAssertLessThanOrEqual(rhoImportance.x, 1.02f);
        XCTAssertEqualWithAccuracy(rhoImportance.x, rhoBrute.x, 0.04f);  // two independent estimators agree

        if (a == 0.05f) {
            // At near-zero roughness the VNDF sampler should closely approach the delta
            // limit: total energy = F + (1-F)/etap^2, NOT ~1. The 1/etap^2 radiance-
            // compression term (see dielectric_eval's transmission branch, and the exact
            // version of this identity in dielectricSmoothTests.mm) means a single
            // interface legitimately loses energy in radiance terms entering a denser
            // medium -- that's real physics, not an energy leak, so "little loss when
            // smooth" was the wrong expectation.
            FresnelResult fr       = fresnel_dielectric(kWo.z, 1.0f, kEta);   // entering: etaI=1, etaT=eta
            float         etap     = kEta;                                    // entering: etap = etaT/etaI = eta
            float         expected = fr.F + (1.0f - fr.F) / (etap * etap);
            XCTAssertEqualWithAccuracy(rhoImportance.x, expected, 0.05f);
        }
    }
}

// A two-lobe pdf (reflection + transmission) integrates to 1 across both hemispheres
- (void)testPdfIntegratesToOneOverFullSphere {
    const float a = 0.3f;
    PdfFn pdf = [a](float3 wo, float3 wi) { return dielectric_pdf(a, kEta, wo, wi); };
    double integral = pdfIntegralFullSphere(pdf, kWo, 22u);
    XCTAssertEqualWithAccuracy(integral, 1.0, 0.05);
}

- (void)testSamplerMatchesOwnPdfChiSquare {
    const float a = 0.3f;
    SampleFn sample = [a, rng = std::mt19937(24u),
                        dist = std::uniform_real_distribution<float>(0.0f, 1.0f)]
                       (float3 wo, float2 u) mutable {
        return dielectric_sample(f3(1, 1, 1), a, kEta, wo, u, dist(rng));
    };
    PdfFn pdf = [a](float3 wo, float3 wi) { return dielectric_pdf(a, kEta, wo, wi); };
    double reducedChi2 = reducedChiSquareFullSphere(sample, pdf, kWo, 25u);
    XCTAssertLessThan(reducedChi2, 3.0);   // heuristic threshold, not a formal p-value test
}

@end

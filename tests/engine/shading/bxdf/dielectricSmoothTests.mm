//
//  dielectricSmoothTests.mm
//  wisteria
//
//  Created by celine on 2026-07-08.
//

#import <XCTest/XCTest.h>
#include <simd/simd.h>
#include "engine/shading/bxdf/Dielectric.h"
#include "../TestUtils.h"

static const simd_float3 kWo  = simd_normalize(f3(0.4f, 0.0f, 0.9f));   // ~24deg off normal, entering
static const float       kEta = 1.5f;                                   // typical glass, relative to air

// throughput = f * |wi.z| / pdf 
static inline float throughputOf(const BSDFSample& bs) {
    return bs.f.x * fabsf(bs.wi.z) / bs.pdf;
}

@interface dielectricSmoothTests : XCTestCase
@end

@implementation dielectricSmoothTests

- (void)testIsDeltaIsSetOnBothBranches {
    BSDFSample reflect = dielectric_smooth_sample(f3(1, 1, 1), kEta, kWo, 0.0f);    // uDiscrete < F always -> reflection
    BSDFSample transmit = dielectric_smooth_sample(f3(1, 1, 1), kEta, kWo, 0.999f);  // F is small off-normal -> refraction
    XCTAssertTrue(reflect.isDelta);
    XCTAssertTrue(transmit.isDelta);
}

// reflection direction is the exact mirror about the shading normal
- (void)testReflectionIsExactMirror {
    BSDFSample bs = dielectric_smooth_sample(f3(1, 1, 1), kEta, kWo, 0.0f);
    simd_float3 mirror = f3(-kWo.x, -kWo.y, kWo.z);
    XCTAssertEqualWithAccuracy(bs.wi.x, mirror.x, 1e-6f);
    XCTAssertEqualWithAccuracy(bs.wi.y, mirror.y, 1e-6f);
    XCTAssertEqualWithAccuracy(bs.wi.z, mirror.z, 1e-6f);
}

// normal incidence never bends, regardless of eta.
- (void)testNormalIncidenceRefractionIsStraightThrough {
    simd_float3 wo = f3(0.0f, 0.0f, 1.0f);
    BSDFSample bs = dielectric_smooth_sample(f3(1, 1, 1), kEta, wo, 0.999f);  // F0 ~= 0.04 << 0.999 -> refraction
    XCTAssertEqualWithAccuracy((double)simd_dot(bs.wi, f3(0.0f, 0.0f, -1.0f)), 1.0, 1e-5);
}

// off-normal refraction direction must match Snell's law exactly
- (void)testRefractionMatchesSnellsLaw {
    simd_float3 wo = simd_normalize(f3(1.0f, 0.0f, 1.0f));   // 45deg off normal, entering
    FresnelResult fr = fresnel_dielectric(wo.z, 1.0f, kEta);   // etaI=1 (air), etaT=eta (entering)
    BSDFSample bs = dielectric_smooth_sample(f3(1, 1, 1), kEta, wo, 0.999f);  // force refraction

    float etaRatio = 1.0f / kEta;
    XCTAssertEqualWithAccuracy(bs.wi.x, -etaRatio * wo.x, 1e-5f);
    XCTAssertEqualWithAccuracy(bs.wi.y, -etaRatio * wo.y, 1e-5f);
    XCTAssertEqualWithAccuracy(bs.wi.z, -fr.cosThetaT, 1e-5f);
    XCTAssertEqualWithAccuracy((double)simd_length(bs.wi), 1.0, 1e-5);   // unit length by construction
}

// pdf is just the discrete lobe-selection probability
- (void)testPdfMatchesFresnelBranchWeight {
    FresnelResult fr = fresnel_dielectric(kWo.z, 1.0f, kEta);
    BSDFSample reflect  = dielectric_smooth_sample(f3(1, 1, 1), kEta, kWo, 0.0f);
    BSDFSample transmit = dielectric_smooth_sample(f3(1, 1, 1), kEta, kWo, 0.999f);
    XCTAssertEqualWithAccuracy(reflect.pdf,  fr.F,        1e-6f);
    XCTAssertEqualWithAccuracy(transmit.pdf, 1.0f - fr.F, 1e-6f);
}

// mirror doesn't attenuate
- (void)testReflectionThroughputIsExactlyOne {
    BSDFSample bs = dielectric_smooth_sample(f3(1, 1, 1), kEta, kWo, 0.0f);
    XCTAssertEqualWithAccuracy(throughputOf(bs), 1.0f, 1e-5f);
}

// transmission throughput is the radiance-compression factor
- (void)testTransmissionThroughputMatchesRadianceCompression {
    BSDFSample bs = dielectric_smooth_sample(f3(1, 1, 1), kEta, kWo, 0.999f);
    float etap = kEta / 1.0f;
    XCTAssertEqualWithAccuracy(throughputOf(bs), 1.0f / (etap * etap), 1e-5f);
}

// energy is conserved entering and leaving
- (void)testRoundTripThroughputRestoresToOne {
    BSDFSample enter = dielectric_smooth_sample(f3(1, 1, 1), kEta, kWo, 0.999f);          // air -> glass

    simd_float3 woExit = f3(kWo.x, kWo.y, -kWo.z);                                        // same angle, glass -> air
    BSDFSample  exit_  = dielectric_smooth_sample(f3(1, 1, 1), kEta, woExit, 0.999f);

    XCTAssertEqualWithAccuracy(throughputOf(enter) * throughputOf(exit_), 1.0f, 1e-4f);
}

// beyond critical angle, F should be 1
- (void)testTotalInternalReflectionAlwaysPicksReflection {
    simd_float3 woSteep = simd_normalize(f3(0.9f, 0.0f, -0.4f));   // ~66deg from normal, exiting glass (> ~41.8deg critical angle for eta=1.5)
    BSDFSample  bs      = dielectric_smooth_sample(f3(1, 1, 1), kEta, woSteep, 0.9999f);

    simd_float3 mirror = f3(-woSteep.x, -woSteep.y, woSteep.z);
    XCTAssertEqualWithAccuracy((double)simd_dot(simd_normalize(bs.wi), mirror), 1.0, 1e-5);
    XCTAssertEqualWithAccuracy(bs.pdf, 1.0f, 1e-5f);
}

@end

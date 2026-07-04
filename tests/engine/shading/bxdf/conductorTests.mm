//
//  conductorTests.mm
//  wisteria
//
//  Host unit tests for the isotropic GGX conductor (engine/shading/bxdf/Conductor.h).
//  Runs off-GPU. Covers Fresnel endpoints, the GGX D normalization, the near-mirror
//  reflection direction, and — the one that catches a measure/Jacobian bug —
//  directional albedo <= 1 (energy conservation, no energy gain).
//

#import <XCTest/XCTest.h>
#include <simd/simd.h>
#include <random>
#include <cmath>
#include "engine/shading/bxdf/Conductor.h"

static inline simd_float3 f3(float x, float y, float z) { return simd_make_float3(x, y, z); }
static inline simd_float2 f2(float x, float y)          { return simd_make_float2(x, y); }

// Uniform direction on the upper hemisphere (pdf = 1/2pi).
static simd_float3 uniformHemisphere(float u1, float u2) {
    float z   = u1;
    float r   = sqrtf(fmaxf(0.0f, 1.0f - z * z));
    float phi = 2.0f * (float)M_PI * u2;
    return f3(r * cosf(phi), r * sinf(phi), z);
}

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
        simd_float3 m = uniformHemisphere(U(rng), U(rng));
        acc += (double)ggx_ndf(m, a) * (double)m.z;
    }
    double integral = acc * (2.0 * M_PI) / N;   // divide out the uniform pdf (1/2pi)
    XCTAssertEqualWithAccuracy(integral, 1.0, 0.03);
}

// Near-zero roughness: the sampled direction is the mirror reflection about +z.
- (void)testNearMirrorDirection {
    simd_float3 wo = simd_normalize(f3(0.3f, 0.0f, 0.9f));
    BSDFSample bs = conductor_sample(f3(1,1,1), 1e-4f, wo, f2(0.5f, 0.5f));
    simd_float3 mirror = f3(-wo.x, -wo.y, wo.z);   // reflect wo about the macronormal
    XCTAssertGreaterThan(bs.pdf, 0.0f);
    XCTAssertGreaterThan((double)simd_dot(bs.wi, mirror), 0.999);
}

// Energy: directional albedo rho(wo) = E[f * cos / pdf] must not exceed 1 (no gain),
// and stays near 1 at low roughness. With F0 = 1 the Fresnel is 1, so rho = E[G2/G1].
- (void)testDirectionalAlbedoNoGain {
    simd_float3 wo = simd_normalize(f3(0.4f, 0.0f, 0.9f));
    const float alphas[] = { 0.05f, 0.2f, 0.5f };
    std::mt19937 rng(2024u);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    const int N = 200000;

    for (float a : alphas) {
        double acc = 0.0;
        for (int i = 0; i < N; ++i) {
            BSDFSample bs = conductor_sample(f3(1,1,1), a, wo, f2(U(rng), U(rng)));
            if (bs.pdf > 0.0f)
                acc += (double)(bs.f.x * fabsf(bs.wi.z) / bs.pdf);   // f * cos / pdf
        }
        double rho = acc / N;
        XCTAssertLessThanOrEqual(rho, 1.01);              // no energy created
        if (a == 0.05f) XCTAssertGreaterThan(rho, 0.9);  // little loss when smooth
    }
}

@end

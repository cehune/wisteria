//
//  estimatorTests.mm
//  wisteria
//
//  Host unit tests for engine/shading/estimator/Estimator.h.
//
//  All *reference* values are computed in double precision -- the estimator is fp32,
//  and comparing fp32 against an fp32 reference would let both be wrong together.
//

#import <XCTest/XCTest.h>
#include <simd/simd.h>
#include <cmath>
#include "engine/shading/estimator/Estimator.h"
#include "../TestUtils.h"

// luminance in double. Test the float 32 values in code to this 
static double lumD(simd_float3 c) {
    return 0.2126 * (double)c.x + 0.7152 * (double)c.y + 0.0722 * (double)c.z;
}

// Two-pass reference: mean of luminance, and the sum of squared luminance deviations with double precision
static void twoPassLum(const simd_float3* s, int N, double& outMeanY, double& outSS) {
    double sum = 0.0;
    // Collect the expected output mean 
    for (int i = 0; i < N; ++i) sum += lumD(s[i]);
    const double mean = sum / (double)N;
    double ss = 0.0;
    
    // Collect the expected output deviation
    for (int i = 0; i < N; ++i) {
        double d = lumD(s[i]) - mean;
        ss += d * d;
    }
    outMeanY = sum / (double)N;
    outSS    = ss;
}

// Fold a whole sequence through the estimator (init on [0], update on the rest).
static EstimatorPixelState runSeq(const simd_float3* s, int N) {
    EstimatorPixelState st = estimator_init(s[0]);
    for (int i = 1; i < N; ++i) st = estimator_update(st, s[i]);
    return st;
}

// Generates a deterministic but varying sample sequence
static void fillDeterministicSampleSeq(simd_float3* s, int N) {
    for (int i = 0; i < N; ++i) {
        float t = (float)i;
        s[i] = f3(0.5f + 0.4f * sinf(0.30f * t),
                  0.5f + 0.3f * sinf(0.11f * t + 1.0f),
                  0.5f + 0.2f * sinf(0.07f * t + 2.0f));
    }
}

@interface estimatorTests : XCTestCase
@end

@implementation estimatorTests

// init seeds exactly, no arithmetic yet, so bit-exact.
- (void)testInitSeedsExactly {
    simd_float3 first = f3(0.2f, 0.5f, 0.9f);
    EstimatorPixelState s = estimator_init(first);
    XCTAssertEqual(s.mu.x, first.x);
    XCTAssertEqual(s.mu.y, first.y);
    XCTAssertEqual(s.mu.z, first.z);
    XCTAssertEqual(s.n,  1.0f);
    XCTAssertEqual(s.m2, 0.0f);
}

// Validate a single update step. 
// 
// The rgb after two samples should only be the mean
// then compare the deviation against the expected double precision values
- (void)testTwoSampleClosedForm {
    simd_float3 a = f3(0.1f, 0.2f, 0.3f);
    simd_float3 b = f3(0.7f, 0.5f, 0.9f);
    EstimatorPixelState s = estimator_update(estimator_init(a), b);

    XCTAssertEqual(s.n, 2.0f);
    XCTAssertEqualWithAccuracy(s.mu.x, 0.5f * (a.x + b.x), 1e-6f);  // exact elementwise average
    XCTAssertEqualWithAccuracy(s.mu.z, 0.5f * (a.z + b.z), 1e-6f);

    double Y1 = lumD(a), Y2 = lumD(b);
    double expected = (Y2 - Y1) * (Y2 - Y1) / 2.0;
    XCTAssertEqualWithAccuracy((double)s.m2, expected, 1e-6);
}

// conformance test: incremental estimator vs the double two-pass over N samples.
- (void)testIncrementalMatchesTwoPass {
    // Generate a bunch of samples
    const int N = 128;
    simd_float3 seq[N];
    fillDeterministicSampleSeq(seq, N);

    // compile all updates through the sample sequences
    EstimatorPixelState s = runSeq(seq, N);
    double meanY, ss;
    twoPassLum(seq, N, meanY, ss); // get the expected double precision mean and deviation

    XCTAssertEqual(s.n, (float)N);
    XCTAssertEqualWithAccuracy(lumD(s.mu), meanY, 1e-5);
    XCTAssertEqualWithAccuracy((double)s.m2, ss, 1e-4 * ss + 1e-6);  // relative tolerance
}

// order independence: a fixed permutation must give the same final mu/m2 within a
// LOOSE tolerance. Do this by shuffling the same set and calculating the rgb and deviation
- (void)testOrderIndependence {
    // Generate a bunch of samples
    const int N = 128;
    simd_float3 seq[N], shuf[N];
    fillDeterministicSampleSeq(seq, N);
    for (int i = 0; i < N; ++i) shuf[i] = seq[(37 * i + 11) % N];

    // both contain the same overal unique set of points
    EstimatorPixelState a = runSeq(seq,  N);
    EstimatorPixelState b = runSeq(shuf, N);

    XCTAssertEqualWithAccuracy(lumD(a.mu), lumD(b.mu), 1e-5);
    double tol = 1e-3 * (double)a.m2 + 1e-5;   // deliberately loose; tight epsilon is flaky here
    XCTAssertEqualWithAccuracy((double)b.m2, (double)a.m2, tol);
}

// constant input (including black): m2 stays EXACTLY 0 and mu never drifts, every step.
// Every subtraction is of identical floating point values, so it should converge
// to exact zero, so exact asserts are justified.
- (void)testConstantInputZeroVariance {
    const int N = 64;
    simd_float3 c = f3(0.42f, 0.17f, 0.88f);
    EstimatorPixelState s = estimator_init(c);
    for (int i = 1; i < N; ++i) {
        s = estimator_update(s, c);
        XCTAssertEqual(s.m2, 0.0f);
        XCTAssertEqual(s.mu.x, c.x);
        XCTAssertEqual(s.mu.y, c.y);
        XCTAssertEqual(s.mu.z, c.z);
    }
    simd_float3 z = f3(0.0f, 0.0f, 0.0f);   // black
    EstimatorPixelState sz = estimator_init(z);
    for (int i = 1; i < N; ++i) {
        sz = estimator_update(sz, z);
        XCTAssertEqual(sz.m2, 0.0f);
    }
}

// m2 is monotonically non-decreasing and non-negative after EVERY update. The Welford
// increment is provably delta^2 * (n-1)/n >= 0, so a single-factor sign flip shows up as a
// transient decrease that a final-state-only check would miss.
- (void)testM2MonotonicNonNegative {
    const int N = 200;
    EstimatorPixelState s = estimator_init(f3(0.3f, 0.3f, 0.3f));
    float prevM2 = s.m2;
    for (int i = 1; i < N; ++i) {
        float base = 0.2f + 0.6f * fabsf(sinf(0.23f * (float)i));
        if (i % 17 == 0) base += 50.0f;                 // firefly mixed into small values
        simd_float3 x = f3(base, base * 0.5f, base * 0.25f);
        s = estimator_update(s, x);
        XCTAssertGreaterThanOrEqual(s.m2, prevM2 - 1e-3f);  // never decreases (fp slack)
        XCTAssertGreaterThanOrEqual(s.m2, 0.0f);            // never negative
        prevM2 = s.m2;
    }
}

// firefly precision characterization: one huge sample among many small ones. 
// need a pretty loose bound because we kind of expect that it dilutes the m2
// THIS IS EXPECTED LOL, thats why its not good on specular paths
- (void)testFireflyStaysFiniteAndTracks {
    const int N = 100;
    simd_float3 seq[N];
    for (int i = 0; i < N; ++i) seq[i] = f3(0.5f, 0.5f, 0.5f);
    seq[50] = f3(1.0e6f, 1.0e6f, 1.0e6f);

    EstimatorPixelState s = runSeq(seq, N);
    XCTAssertTrue(std::isfinite(s.m2));
    XCTAssertTrue(std::isfinite(s.mu.x));

    double meanY, ss;
    twoPassLum(seq, N, meanY, ss);
    XCTAssertEqualWithAccuracy((double)s.m2, ss,   0.02 * ss);      // within 2%
    XCTAssertEqualWithAccuracy(lumD(s.mu),   meanY, 1e-2 * meanY);
}

// luminance() weights (calculated by hand)
- (void)testLuminanceWeights {
    XCTAssertEqualWithAccuracy(luminance(f3(1.0f, 0.0f, 0.0f)), 0.2126f, 1e-6f);
    XCTAssertEqualWithAccuracy(luminance(f3(0.0f, 1.0f, 0.0f)), 0.7152f, 1e-6f);
    XCTAssertEqualWithAccuracy(luminance(f3(0.0f, 0.0f, 1.0f)), 0.0722f, 1e-6f);
    XCTAssertEqual(luminance(f3(0.0f, 0.0f, 0.0f)), 0.0f);
    // not exact bc floating point is stupid with div
    XCTAssertEqualWithAccuracy(luminance(f3(1.0f, 1.0f, 1.0f)), 1.0f, 1e-6f);
}

// standardError bounded by the luminance mean
- (void)testStandardErrorBoundary {
    EstimatorPixelState s1 = estimator_init(f3(0.5f, 0.5f, 0.5f));
    XCTAssertEqual(estimator_standardError(s1), 1.0f);
    XCTAssertFalse(std::isnan(estimator_standardError(s1)));

    simd_float3 a = f3(0.2f, 0.2f, 0.2f), b = f3(0.8f, 0.8f, 0.8f);
    EstimatorPixelState s2 = estimator_update(estimator_init(a), b);
    double expectedSE = std::fabs(lumD(b) - lumD(a)) / 2.0;
    XCTAssertEqualWithAccuracy((double)estimator_standardError(s2), expectedSE, 1e-6);
}

@end

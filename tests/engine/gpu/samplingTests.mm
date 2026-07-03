//
//  samplingTests.mm
//  wisteria
//
//  Host unit tests for Sampling.h — the power heuristic and the mode-gated MIS
//  weight. Pure float logic, no GPU.
//

#import <XCTest/XCTest.h>
#include "engine/shading/common/Sampling.h"

@interface samplingTests : XCTestCase
@end

@implementation samplingTests

// 2^2 / (2^2 + 1^2) = 4/5
- (void)testPowerHeuristicKnownValue {
    XCTAssertEqualWithAccuracy(powerHeuristic(2.0f, 1.0f), 0.8f, 1e-6f);
}

- (void)testPowerHeuristicEqualPdfsHalf {
    XCTAssertEqualWithAccuracy(powerHeuristic(4.0f, 4.0f), 0.5f, 1e-6f);
}

- (void)testPowerHeuristicDegenerate {
    XCTAssertEqual(powerHeuristic(0.0f, 0.0f), 0.0f);                       // guard, no NaN
    XCTAssertEqualWithAccuracy(powerHeuristic(1.0f, 0.0f), 1.0f, 1e-6f);    // other strategy useless
    XCTAssertEqualWithAccuracy(powerHeuristic(0.0f, 5.0f), 0.0f, 1e-6f);
}

// The invariant that keeps MIS unbiased: for a given (pLight, pBsdf) the light-term
// and the matching bsdf-term weights sum to 1.
- (void)testMisWeightsPartition {
    float pL = 2.0f, pB = 7.0f;
    float wL = misWeight(pL, pB, STRATEGY_LIGHT, INTEGRATOR_MIS);
    float wB = misWeight(pB, pL, STRATEGY_BSDF,  INTEGRATOR_MIS);
    XCTAssertEqualWithAccuracy(wL + wB, 1.0f, 1e-6f);
}

- (void)testMisModeMatchesPowerHeuristic {
    XCTAssertEqualWithAccuracy(misWeight(2.0f, 1.0f, STRATEGY_LIGHT, INTEGRATOR_MIS),
                               powerHeuristic(2.0f, 1.0f), 1e-6f);
}

- (void)testNeeOnlyKeepsLightDropsBsdf {
    XCTAssertEqualWithAccuracy(misWeight(2.0f, 7.0f, STRATEGY_LIGHT, INTEGRATOR_NEE), 1.0f, 1e-6f);
    XCTAssertEqualWithAccuracy(misWeight(2.0f, 7.0f, STRATEGY_BSDF,  INTEGRATOR_NEE), 0.0f, 1e-6f);
}

- (void)testBsdfOnlyKeepsBsdfDropsLight {
    XCTAssertEqualWithAccuracy(misWeight(2.0f, 7.0f, STRATEGY_BSDF,  INTEGRATOR_BSDF), 1.0f, 1e-6f);
    XCTAssertEqualWithAccuracy(misWeight(2.0f, 7.0f, STRATEGY_LIGHT, INTEGRATOR_BSDF), 0.0f, 1e-6f);
}

@end

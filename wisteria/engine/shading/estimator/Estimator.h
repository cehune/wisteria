//
//  Estimator.h
//  wisteria
//
//  Created by celine on 2026-07-28.
//
#pragma once
#include "../common/Common.h"
#include "../common/Spectrum.h"
using namespace wst;

// Running state for one pixel
struct EstimatorPixelState {
  float3 mu; // RGB mean
  float n;   // sample count
  float m2;  // Welford sum of squared luminance deviations (NOT E[Y^2])
};

// first sample of a run, with zero deviation by definition
inline EstimatorPixelState estimator_init(float3 firstSample) {
  return EstimatorPixelState{ firstSample, 1.0f, 0.0f };
}

// Fold another sample into the current state.
// Welford's online algorithm 
// Luminance is linear in RGB, so the running luminance mean is just luminance(mu):
// no separate channel to store, only the 4 extra bytes of m2.
//
// NOTE: fireflies can make deviation huge, unit scale updates
// will lose bits. TODO: Address once we do specular and caustic paths
inline EstimatorPixelState estimator_update(EstimatorPixelState prev, float3 sample) {
  EstimatorPixelState next;

  next.n  = prev.n + 1.0f;
  next.mu = prev.mu + (sample - prev.mu) / next.n;

  float Ys        = luminance(sample);
  float muPrevLum = luminance(prev.mu);   // mean-of-luma BEFORE update (= luma(mean), by linearity)
  float muNextLum = luminance(next.mu);   // mean-of-luma AFTER update
  next.m2 = prev.m2 + (Ys - muPrevLum) * (Ys - muNextLum);   // Welford: delta * delta2

  return next;
}

// Standard error of the luminance mean estimate. sqrt( m2 / (n (n-1)) ) = sqrt(sigma^2 / n).
// This is ABSOLUTE (luminance units) and scales with brightness. An adaptive
// stopping rule should compare RELATIVE error instead (ie bright and dark pixels have same
// relative noise floor) TODO: Address when we do adaptive stopping in rendering
inline float estimator_standardError(EstimatorPixelState s) {
    return (s.n > 1.0f) ? sqrt(s.m2 / (s.n * (s.n - 1.0f))) : 1.0f;
}

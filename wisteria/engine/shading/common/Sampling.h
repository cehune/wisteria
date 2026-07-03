//
//  Sampling.h
//  wisteria
//
//  Created by celine on 2026-07-02.
//
//  The lot for MIS!!!  Pure float logic — no vector types, host-testable.

#pragma once

// Veaches power heuristic
inline float powerHeuristic(float pdfA, float pdfB) {
    float a2 = pdfA * pdfA, b2 = pdfB * pdfB;
    float d  = a2 + b2;
    return d > 0.0f ? a2 / d : 0.0f;
}

// Direct-lighting integrator mode. NEE and BSDF render the SAME image as MIS at
// convergence (only variance differs) — they're the validation baselines.
enum IntegratorMode : int { INTEGRATOR_NEE = 0, INTEGRATOR_BSDF = 1, INTEGRATOR_MIS = 2 };

// Which strategy produced a direct-lighting term.
enum LightStrategy : int { STRATEGY_LIGHT = 0, STRATEGY_BSDF = 1 };

// Weight for a direct-lighting term drawn from `strategy`, given its own solid-angle
// pdf (pSelf) and the pdf the other strategy would assign the same direction (pOther).
// MIS -> power heuristic; a single-strategy mode keeps the term (1) iff it came from
// the active strategy, else drops it (0). Camera / post-specular hits bypass this
// (always weight 1) — no strategy pairing applies there.
inline float misWeight(float pSelf, float pOther, int strategy, int mode) {
    if (mode == INTEGRATOR_MIS) return powerHeuristic(pSelf, pOther);
    bool active = (mode == INTEGRATOR_NEE  && strategy == STRATEGY_LIGHT)
               || (mode == INTEGRATOR_BSDF && strategy == STRATEGY_BSDF);
    return active ? 1.0f : 0.0f;
}

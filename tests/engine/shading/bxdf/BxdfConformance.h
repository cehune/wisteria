//
//  BxdfConformance.h
//  wisteria
//
//  Created by celine on 2026-07-08.
//

#pragma once
#include <random>
#include <cmath>
#include <functional>
#include <vector>
#include <algorithm>
#include "engine/shading/common/Common.h"
#include "engine/shading/bxdf/BsdfSample.h"

using namespace wst;

namespace bxdftest {

using SampleFn = std::function<BSDFSample(float3 wo, float2 u)>;
using EvalFn   = std::function<float3(float3 wo, float3 wi)>;
using PdfFn    = std::function<float(float3 wo, float3 wi)>;

// Uniform direction on the upper hemisphere; pdf = 1/(2*Pi) in solid angle.
inline float3 uniformHemisphere(float u1, float u2) {
    float z     = u1;
    float r     = std::sqrt(std::max(0.0f, 1.0f - z * z));
    float phi   = 2.0f * Pi * u2;
    return float3{ r * std::cos(phi), r * std::sin(phi), z };
}

// Uniform direction on the full sphere; pdf = 1/(4*Pi) in solid angle. Needed for BxDFs
// with a transmission lobe (e.g. a rough dielectric), where wi can land in either
// hemisphere and a hemisphere-only estimator silently drops the BTDF entirely.
inline float3 uniformSphere(float u1, float u2) {
    float z     = 2.0f * u1 - 1.0f;
    float r     = std::sqrt(std::max(0.0f, 1.0f - z * z));
    float phi   = 2.0f * Pi * u2;
    return float3{ r * std::cos(phi), r * std::sin(phi), z };
}

// Directional-hemispherical reflectance rho(wo), importance-sampled through `sample`.
inline float3 directionalAlbedoImportance(const SampleFn& sample, float3 wo, uint32_t seed, int N = 200000) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    double accR = 0.0, accG = 0.0, accB = 0.0;
    for (int i = 0; i < N; ++i) {
        BSDFSample bs = sample(wo, float2{ U(rng), U(rng) });
        if (bs.pdf > 0.0f) {
            double w = (double)std::fabs(bs.wi.z) / (double)bs.pdf;
            accR += (double)bs.f.x * w;
            accG += (double)bs.f.y * w;
            accB += (double)bs.f.z * w;
        }
    }
    return float3{ (float)(accR / N), (float)(accG / N), (float)(accB / N) };
}

// Same integral via brute-force uniform-hemisphere sampling of `eval` alone — an
// estimator that never touches `sample`/`pdf`. Disagreement with the importance-sampled
// version above means sample/pdf don't actually correspond to eval.
inline float3 directionalAlbedoBruteForce(const EvalFn& eval, float3 wo, uint32_t seed, int N = 400000) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    const double uniformPdf = 1.0 / (2.0 * (double)Pi);
    double accR = 0.0, accG = 0.0, accB = 0.0;
    for (int i = 0; i < N; ++i) {
        float3 wi = uniformHemisphere(U(rng), U(rng));
        float3 f  = eval(wo, wi);
        double w  = (double)std::fabs(wi.z) / uniformPdf;
        accR += (double)f.x * w;
        accG += (double)f.y * w;
        accB += (double)f.z * w;
    }
    return float3{ (float)(accR / N), (float)(accG / N), (float)(accB / N) };
}

// integral of pdf(wo, wi) dwi over the upper hemisphere, via uniform-hemisphere sampling.
// Must equal 1 for any properly normalized reflection pdf.
inline double pdfIntegral(const PdfFn& pdf, float3 wo, uint32_t seed, int N = 400000) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    const double uniformPdf = 1.0 / (2.0 * (double)Pi);
    double acc = 0.0;
    for (int i = 0; i < N; ++i) {
        float3 wi = uniformHemisphere(U(rng), U(rng));
        acc += (double)pdf(wo, wi) / uniformPdf;
    }
    return acc / N;
}

// Full-sphere variant of directionalAlbedoBruteForce: for BxDFs with a transmission
// lobe, eval(wo, wi) is only ever non-zero for a hemisphere-only wi if the BxDF is
// reflection-only. A rough dielectric needs wi drawn from the full sphere to see the
// BTDF half of eval at all.
inline float3 directionalAlbedoBruteForceFullSphere(const EvalFn& eval, float3 wo, uint32_t seed, int N = 400000) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    const double uniformPdf = 1.0 / (4.0 * (double)Pi);
    double accR = 0.0, accG = 0.0, accB = 0.0;
    for (int i = 0; i < N; ++i) {
        float3 wi = uniformSphere(U(rng), U(rng));
        float3 f  = eval(wo, wi);
        double w  = (double)std::fabs(wi.z) / uniformPdf;
        accR += (double)f.x * w;
        accG += (double)f.y * w;
        accB += (double)f.z * w;
    }
    return float3{ (float)(accR / N), (float)(accG / N), (float)(accB / N) };
}

// Full-sphere variant of pdfIntegral. A two-lobe pdf (reflection + transmission, split
// stochastically by Fresnel weight) only integrates to 1 across both hemispheres together
// — each lobe alone integrates to less than 1.
inline double pdfIntegralFullSphere(const PdfFn& pdf, float3 wo, uint32_t seed, int N = 400000) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    const double uniformPdf = 1.0 / (4.0 * (double)Pi);
    double acc = 0.0;
    for (int i = 0; i < N; ++i) {
        float3 wi = uniformSphere(U(rng), U(rng));
        acc += (double)pdf(wo, wi) / uniformPdf;
    }
    return acc / N;
}

// max_i |f(wo,wi) - f(wi,wo)| over random (wo,wi) pairs on the upper hemisphere. Only
// meaningful for reciprocal BxDFs (reflection-only, equal IOR both sides) — a rough
// dielectric's BTDF needs the eta^2 radiance-compression term and must not use this.
inline float maxReciprocityError(const EvalFn& eval, uint32_t seed, int pairs = 20000) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);
    float worst = 0.0f;
    for (int i = 0; i < pairs; ++i) {
        float3 wo = uniformHemisphere(U(rng), U(rng));
        float3 wi = uniformHemisphere(U(rng), U(rng));
        float3 a  = eval(wo, wi);
        float3 b  = eval(wi, wo);
        worst = std::max({ worst, std::fabs(a.x - b.x), std::fabs(a.y - b.y), std::fabs(a.z - b.z) });
    }
    return worst;
}

// Reduced chi-squared (chi2 / degrees-of-freedom) between `sample`'s empirical direction
// histogram and the density `pdf` predicts, binned on a (cosTheta, phi) grid — that
// parameterization gives every bin equal solid angle (dOmega = -dcosTheta dphi), so a
// bin's probability mass is just (mean pdf in bin) * (1 / numBins). A value near 1 means
// the sampler's output distribution matches its own reported pdf. Not a formal p-value
// test (no critical-value table) — treat the threshold as a heuristic, not a proof.
//
// Precondition: wo.z > 0 (matches this codebase's convention that wo is already flipped
// into the upper hemisphere before reaching a BxDF).
inline double reducedChiSquare(const SampleFn& sample, const PdfFn& pdf, float3 wo,
                                uint32_t seed, int N = 200000,
                                int thetaBins = 8, int phiBins = 16, int subSamplesPerBin = 64) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);

    const int numBins = thetaBins * phiBins;
    std::vector<double> observed(numBins, 0.0);
    std::vector<double> expectedDensity(numBins, 0.0); // mean pdf within the bin

    int drawn = 0;
    for (int i = 0; i < N; ++i) {
        BSDFSample bs = sample(wo, float2{ U(rng), U(rng) });
        if (bs.pdf <= 0.0f) continue;
        float cosT = std::min(1.0f, std::max(0.0f, bs.wi.z));
        float phi  = std::atan2(bs.wi.y, bs.wi.x);
        if (phi < 0.0f) phi += 2.0f * Pi;
        int tb = std::min(thetaBins - 1, (int)(cosT * thetaBins));
        int pb = std::min(phiBins - 1, (int)(phi / (2.0f * Pi) * phiBins));
        observed[tb * phiBins + pb] += 1.0;
        ++drawn;
    }

    const double binSolidAngle = (2.0 * (double)Pi) / numBins;
    for (int tb = 0; tb < thetaBins; ++tb) {
        for (int pb = 0; pb < phiBins; ++pb) {
            double acc = 0.0;
            for (int s = 0; s < subSamplesPerBin; ++s) {
                float cosT = (tb + U(rng)) / thetaBins;
                float sinT = std::sqrt(std::max(0.0f, 1.0f - cosT * cosT));
                float phi  = (pb + U(rng)) / phiBins * 2.0f * Pi;
                float3 wi{ sinT * std::cos(phi), sinT * std::sin(phi), cosT };
                acc += (double)pdf(wo, wi);
            }
            expectedDensity[tb * phiBins + pb] = acc / subSamplesPerBin;
        }
    }

    double chi2 = 0.0;
    int dfBins = 0;
    for (int i = 0; i < numBins; ++i) {
        double expected = expectedDensity[i] * binSolidAngle * drawn;
        if (expected < 5.0) continue;   // Cochran's rule: skip sparse bins
        double diff = observed[i] - expected;
        chi2 += diff * diff / expected;
        ++dfBins;
    }
    return dfBins > 1 ? chi2 / (double)(dfBins - 1) : 0.0;
}

// Full-sphere variant of reducedChiSquare, for BxDFs whose samples can land in either
// hemisphere (e.g. a rough dielectric's transmission lobe). Bins cosTheta over [-1, 1]
// instead of clamping negative-hemisphere samples into the theta=0 bin.
inline double reducedChiSquareFullSphere(const SampleFn& sample, const PdfFn& pdf, float3 wo,
                                          uint32_t seed, int N = 200000,
                                          int thetaBins = 16, int phiBins = 16, int subSamplesPerBin = 64) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> U(0.0f, 1.0f);

    const int numBins = thetaBins * phiBins;
    std::vector<double> observed(numBins, 0.0);
    std::vector<double> expectedDensity(numBins, 0.0); // mean pdf within the bin

    int drawn = 0;
    for (int i = 0; i < N; ++i) {
        BSDFSample bs = sample(wo, float2{ U(rng), U(rng) });
        if (bs.pdf <= 0.0f) continue;
        float cosT = std::min(1.0f, std::max(-1.0f, bs.wi.z));
        float phi  = std::atan2(bs.wi.y, bs.wi.x);
        if (phi < 0.0f) phi += 2.0f * Pi;
        int tb = std::min(thetaBins - 1, (int)((cosT + 1.0f) * 0.5f * thetaBins));
        int pb = std::min(phiBins - 1, (int)(phi / (2.0f * Pi) * phiBins));
        observed[tb * phiBins + pb] += 1.0;
        ++drawn;
    }

    const double binSolidAngle = (4.0 * (double)Pi) / numBins;
    for (int tb = 0; tb < thetaBins; ++tb) {
        for (int pb = 0; pb < phiBins; ++pb) {
            double acc = 0.0;
            for (int s = 0; s < subSamplesPerBin; ++s) {
                float cosT = -1.0f + 2.0f * (tb + U(rng)) / thetaBins;
                float sinT = std::sqrt(std::max(0.0f, 1.0f - cosT * cosT));
                float phi  = (pb + U(rng)) / phiBins * 2.0f * Pi;
                float3 wi{ sinT * std::cos(phi), sinT * std::sin(phi), cosT };
                acc += (double)pdf(wo, wi);
            }
            expectedDensity[tb * phiBins + pb] = acc / subSamplesPerBin;
        }
    }

    double chi2 = 0.0;
    int dfBins = 0;
    for (int i = 0; i < numBins; ++i) {
        double expected = expectedDensity[i] * binSolidAngle * drawn;
        if (expected < 5.0) continue;   // Cochran's rule: skip sparse bins
        double diff = observed[i] - expected;
        chi2 += diff * diff / expected;
        ++dfBins;
    }
    return dfBins > 1 ? chi2 / (double)(dfBins - 1) : 0.0;
}

} // namespace bxdftest

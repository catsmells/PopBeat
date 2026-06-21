#include "FilterSVF.h"
#include <algorithm>
#include <cmath>
void FilterSVF::setSampleRate(float sr) {
    sampleRate_ = sr;
    cutoffSmoother_.setTimeMs(8.0f, sr);
    resonanceSmoother_.setTimeMs(8.0f, sr);
    cutoffSmoother_.snap(cutoffTarget_);
    resonanceSmoother_.snap(resonanceTarget_);
}
void FilterSVF::updateCoefficients(float cutoffHz, float resonance) {
    const float clampedCutoff =
        std::clamp(cutoffHz, 20.0f, sampleRate_ * 0.45f);
    const float q = 0.5f + std::clamp(resonance, 0.0f, 1.0f) * 19.5f;
    k_ = 1.0f / q;
    g_ = std::tan(3.14159265358979f * clampedCutoff / sampleRate_);
    a1_ = 1.0f / (1.0f + g_ * (g_ + k_));
    a2_ = g_ * a1_;
    a3_ = g_ * a2_;
}
float FilterSVF::process(float input) {
    const float resonance = resonanceSmoother_.process(resonanceTarget_);
    const float cutoff = cutoffSmoother_.process(cutoffTarget_);
    updateCoefficients(cutoff, resonance);
    const float v3 = input - ic2eq_;
    const float v1 = a1_ * ic1eq_ + a2_ * v3;
    const float v2 = ic2eq_ + a2_ * ic1eq_ + a3_ * v3;
    ic1eq_ = 2.0f * v1 - ic1eq_;
    ic2eq_ = 2.0f * v2 - ic2eq_;
    float out;
    switch (mode_) {
        case SvfMode::LowPass:
            out = v2;
            break;
        case SvfMode::HighPass:
            out = input - k_ * v1 - v2;
            break;
        case SvfMode::BandPass:
        default:
            out = v1;
            break;
    }
    out = out / (1.0f + std::fabs(out));
    if (!std::isfinite(out) || !std::isfinite(ic1eq_) || !std::isfinite(ic2eq_)) {
        ic1eq_ = 0.0f;
        ic2eq_ = 0.0f;
        out = 0.0f;
    }
    return(out);
}

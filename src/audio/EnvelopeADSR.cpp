#include "EnvelopeADSR.h"
#include <cmath>
namespace {
float rateFor(float ms, float sampleRate) {
    if (ms <= 0.0f) return 1.0f;
    return 1.0f - std::exp(-1.0f / (0.001f * ms * sampleRate));
}
}
float EnvelopeADSR::process() {
    switch (stage_) {
        case AdsrStage::Idle:
            level_ = 0.0f;
            break;
        case AdsrStage::Attack: {
            const float rate = rateFor(attackMs_, sampleRate_);
            level_ += (1.0f - level_) * rate;
            if (level_ >= 0.999f) {
                level_ = 1.0f;
                stage_ = AdsrStage::Decay;
            }
            break;
        }
        case AdsrStage::Decay: {
            const float rate = rateFor(decayMs_, sampleRate_);
            level_ += (sustain_ - level_) * rate;
            if (std::fabs(level_ - sustain_) <= 0.0005f) {
                level_ = sustain_;
                stage_ = AdsrStage::Sustain;
            }
            break;
        }
        case AdsrStage::Sustain:
            level_ = sustain_;
            break;
        case AdsrStage::Release: {
            const float rate = rateFor(releaseMs_, sampleRate_);
            level_ += (0.0f - level_) * rate;
            if (level_ <= 0.0005f) {
                level_ = 0.0f;
                stage_ = AdsrStage::Idle;
            }
            break;
        }
    }
    return level_;
}

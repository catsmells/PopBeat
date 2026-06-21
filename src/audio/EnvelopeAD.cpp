#include "EnvelopeAD.h"
#include <cmath>
namespace {
float rateFor(float ms, float sampleRate) {
    if (ms <= 0.0f) return 1.0f;
    return 1.0f - std::exp(-1.0f / (0.001f * ms * sampleRate));}}
float EnvelopeAD::process() {
    switch (stage_) {
        case EgStage::Idle:
            level_ = 0.0f;
            break;
        case EgStage::Attack: {
            const float rate = rateFor(attackMs_, sampleRate_);
            level_ += (1.0f - level_) * rate;
            if (level_ >= 0.999f) {
                level_ = 1.0f;
                stage_ = EgStage::Decay;
            }
            break;
        }
        case EgStage::Decay: {
            const float rate = rateFor(decayMs_, sampleRate_);
            level_ += (0.0f - level_) * rate;
            if (level_ <= 0.0005f) {
                level_ = 0.0f;
                stage_ = EgStage::Idle;
            }
            break;
        }
    }
    return level_;
}

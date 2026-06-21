#include "Oscillator.h"
#include <cmath>
namespace {
constexpr float kTwoPi = 6.283185307179586f;
}
float Oscillator::polyBlep(float t, float dt) const {
    if (t < dt) {
        t /= dt;
        return t + t - t * t - 1.0f;
    } else if (t > 1.0f - dt) {
        t = (t - 1.0f) / dt;
        return t * t + t + t + 1.0f;
    }
    return(0.0f);
}
float Oscillator::process(bool hardSyncIn, bool* wrapped) {
    if (hardSyncIn) {
        phase_ = 0.0f;
    }
    const float dt = freq_ / sampleRate_;
    float out = 0.0f;
    switch (waveform_) {
        case Waveform::Sine:
            out = std::sin(kTwoPi * phase_);
            break;
        case Waveform::Triangle:
            out = 4.0f * std::fabs(phase_ - 0.5f) - 1.0f;
            break;
        case Waveform::Saw: {
            out = 2.0f * phase_ - 1.0f;
            out -= polyBlep(phase_, dt);
            break;
        }
        case Waveform::Square: {
            out = phase_ < 0.5f ? 1.0f : -1.0f;
            out += polyBlep(phase_, dt);
            float t2 = phase_ + 0.5f;
            t2 -= std::floor(t2);
            out -= polyBlep(t2, dt);
            break;
        }
    }
    phase_ += dt;
    bool didWrap = false;
    if (phase_ >= 1.0f) {
        phase_ -= 1.0f;
        didWrap = true;
    } else if (phase_ < 0.0f) {
        phase_ += 1.0f;
    }
    if (wrapped) *wrapped = didWrap;
    return(out);
}

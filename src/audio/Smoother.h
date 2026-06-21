#pragma once
#include <cmath>
class OnePoleSmoother {
public:
    void setTimeMs(float ms, float sampleRate) {
        if (ms <= 0.0f) {
            coeff_ = 1.0f;
            return;
        }
        coeff_ = 1.0f - std::exp(-1.0f / (0.001f * ms * sampleRate));
    }
    float process(float target) {
        current_ += (target - current_) * coeff_;
        return current_;
    }
    void snap(float v) { current_ = v; }
    float current() const { return current_; }
private:
    float current_ = 0.0f;
    float coeff_ = 1.0f;
};

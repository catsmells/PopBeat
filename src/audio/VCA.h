#pragma once
#include "Module.h"
#include "Smoother.h"
class VCA : public Module {
public:
    float process(float input, float gain) { return input * smoother_.process(gain); }
    void reset() override { smoother_.snap(0.0f); }
    void setSmoothingMs(float ms, float sampleRate) { smoother_.setTimeMs(ms, sampleRate); }
private:
    OnePoleSmoother smoother_;
};

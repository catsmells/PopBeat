#pragma once
#include "Module.h"
#include "Smoother.h"
enum class SvfMode { LowPass, HighPass, BandPass };
class FilterSVF : public Module {
public:
    void setSampleRate(float sr);
    void setMode(SvfMode m) { mode_ = m; }
    void setCutoff(float hz) { cutoffTarget_ = hz; }
    void setResonance(float r) { resonanceTarget_ = r; }
    float process(float input);
    void reset() override {
        ic1eq_ = 0.0f;
        ic2eq_ = 0.0f;
    }
private:
    void updateCoefficients(float cutoffHz, float resonance);
    SvfMode mode_ = SvfMode::LowPass;
    float sampleRate_ = 44100.0f;
    float cutoffTarget_ = 1000.0f;
    float resonanceTarget_ = 0.2f;
    OnePoleSmoother cutoffSmoother_;
    OnePoleSmoother resonanceSmoother_;
    float ic1eq_ = 0.0f;
    float ic2eq_ = 0.0f;
    float g_ = 0.0f;
    float k_ = 0.0f;
    float a1_ = 0.0f;
    float a2_ = 0.0f;
    float a3_ = 0.0f;
};

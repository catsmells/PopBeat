#pragma once
#include "Module.h"
#include "Oscillator.h"
class LFO : public Module {
public:
    void setSampleRate(float sr) { osc_.setSampleRate(sr); }
    void setWaveform(Waveform w) { osc_.setWaveform(w); }
    void setRateHz(float hz) { osc_.setFrequency(hz); }
    float process() { return osc_.process(false); }
    void reset() override { osc_.reset(); }
private:
    Oscillator osc_{};
};

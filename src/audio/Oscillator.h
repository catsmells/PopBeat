#pragma once
#include "Module.h"
enum class Waveform { Sine, Triangle, Saw, Square };
class Oscillator : public Module {
public:
    void setSampleRate(float sr) { sampleRate_ = sr; }
    void setWaveform(Waveform w) { waveform_ = w; }
    void setFrequency(float hz) { freq_ = hz; }
    float process(bool hardSyncIn, bool* wrapped = nullptr);
    void reset() override { phase_ = 0.0f; }
private:
    float polyBlep(float t, float dt) const;
    float phase_ = 0.0f;
    float freq_ = 440.0f;
    float sampleRate_ = 44100.0f;
    Waveform waveform_ = Waveform::Saw;
};

#pragma once
#include "Module.h"
enum class AdsrStage { Idle, Attack, Decay, Sustain, Release };
class EnvelopeADSR : public Module {
public:
    void setSampleRate(float sr) { sampleRate_ = sr; }
    void setAttackMs(float ms) { attackMs_ = ms; }
    void setDecayMs(float ms) { decayMs_ = ms; }
    void setSustain(float level) { sustain_ = level; }
    void setReleaseMs(float ms) { releaseMs_ = ms; }
    void noteOn() { stage_ = AdsrStage::Attack; }
    void noteOff() { stage_ = AdsrStage::Release; }
    float process();
    void reset() override {
        stage_ = AdsrStage::Idle;
        level_ = 0.0f;
    }
private:
    AdsrStage stage_ = AdsrStage::Idle;
    float level_ = 0.0f;
    float attackMs_ = 5.0f;
    float decayMs_ = 100.0f;
    float sustain_ = 0.7f;
    float releaseMs_ = 200.0f;
    float sampleRate_ = 44100.0f;
};

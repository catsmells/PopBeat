#pragma once
#include "Module.h"
enum class EgStage { Idle, Attack, Decay };
class EnvelopeAD : public Module {
public:
    void setSampleRate(float sr) { sampleRate_ = sr; }
    void setAttackMs(float ms) { attackMs_ = ms; }
    void setDecayMs(float ms) { decayMs_ = ms; }
    void trigger() { stage_ = EgStage::Attack; }
    float process();
    void reset() override {
        stage_ = EgStage::Idle;
        level_ = 0.0f;
    }
private:
    EgStage stage_ = EgStage::Idle;
    float level_ = 0.0f;
    float attackMs_ = 2.0f;
    float decayMs_ = 200.0f;
    float sampleRate_ = 44100.0f;
};

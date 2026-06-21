#pragma once
#include <atomic>
#include <vector>
#include "../audio/EnvelopeAD.h"
#include "../audio/FilterSVF.h"
#include "../audio/NoiseSource.h"
#include "../audio/Oscillator.h"
#include "../audio/Param.h"
#include "../audio/StepSequencer.h"
#include "../audio/VCA.h"
class DfamVoice {
public:
    DfamVoice();
    void setSampleRate(float sr);
    float process();
    void reset();
    StepSequencer& sequencer() { return seq_; }
    const std::vector<Param>& params() const { return params_; }
    std::atomic<float> vco1Tune{0.0f};
    std::atomic<float> vco2Tune{0.0f};
    std::atomic<float> mixVco1{0.7f};
    std::atomic<float> mixVco2{0.0f};
    std::atomic<float> mixNoise{0.0f};
    std::atomic<float> filterCutoff{800.0f};
    std::atomic<float> filterResonance{0.3f};
    std::atomic<float> eg1Attack{2.0f};
    std::atomic<float> eg1Decay{150.0f};
    std::atomic<float> eg2Attack{2.0f};
    std::atomic<float> eg2Decay{300.0f};
    std::atomic<float> eg2PitchModAmount{0.0f};
    std::atomic<float> eg2FilterModAmount{0.0f};
    void clearExtInputs() {
        extVco1Pitch_ = 0.0f;
        extVco2Pitch_ = 0.0f;
        extFilterCv_ = 0.0f;
        extTrigIn_ = 0.0f;
    }
    void addExtVco1Pitch(float semis) { extVco1Pitch_ += semis; }
    void addExtVco2Pitch(float semis) { extVco2Pitch_ += semis; }
    void addExtFilterCv(float hz) { extFilterCv_ += hz; }
    void addExtTrigIn(float v) { extTrigIn_ += v; }
    float lastVco1() const { return lastVco1_; }
    float lastVco2() const { return lastVco2_; }
    float lastNoise() const { return lastNoise_; }
    float lastEg1() const { return lastEg1_; }
    float lastEg2() const { return lastEg2_; }
    float lastFilterOut() const { return lastFilterOut_; }
    float lastTrig() const { return lastTrig_; }
private:
    Oscillator vco1_;
    Oscillator vco2_;
    NoiseSource noise_;
    FilterSVF filter_;
    EnvelopeAD eg1_;
    EnvelopeAD eg2_;
    VCA vca_;
    StepSequencer seq_;
    std::vector<Param> params_;
    float sampleRate_ = 44100.0f;
    float extVco1Pitch_ = 0.0f;
    float extVco2Pitch_ = 0.0f;
    float extFilterCv_ = 0.0f;
    float extTrigIn_ = 0.0f;
    bool extTrigInPrev_ = false;
    float lastVco1_ = 0.0f;
    float lastVco2_ = 0.0f;
    float lastNoise_ = 0.0f;
    float lastEg1_ = 0.0f;
    float lastEg2_ = 0.0f;
    float lastFilterOut_ = 0.0f;
    float lastTrig_ = 0.0f;
    int trigSamplesRemaining_ = 0;
    static constexpr int kTrigPulseSamples = 441;
    static constexpr float kBaseHz = 55.0f;
};

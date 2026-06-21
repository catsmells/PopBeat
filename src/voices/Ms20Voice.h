#pragma once
#include <atomic>
#include <vector>
#include "../audio/EnvelopeADSR.h"
#include "../audio/FilterSVF.h"
#include "../audio/LFO.h"
#include "../audio/Oscillator.h"
#include "../audio/Param.h"
#include "../audio/VCA.h"
class Ms20Voice {
public:
    Ms20Voice();
    void setSampleRate(float sr);
    float process();
    void reset();
    void noteOn(int midiNote);
    void noteOff(int midiNote);
    void setExternalTempo(float bpm) { externalTempo_.store(bpm, std::memory_order_relaxed); }
    const std::vector<Param>& params() const { return params_; }
    std::atomic<float> vco1Tune{0.0f};
    std::atomic<float> vco2Tune{-12.0f};
    std::atomic<float> mixVco1{0.7f};
    std::atomic<float> mixVco2{0.3f};
    std::atomic<float> mixRingMod{0.0f};
    std::atomic<bool> hardSyncEnabled{false};
    std::atomic<float> hpfCutoff{80.0f};
    std::atomic<float> lpfCutoff{2000.0f};
    std::atomic<float> lpfResonance{0.3f};
    std::atomic<float> egAttack{5.0f};
    std::atomic<float> egDecay{100.0f};
    std::atomic<float> egSustain{0.7f};
    std::atomic<float> egRelease{200.0f};
    std::atomic<float> lfoRate{4.0f};
    std::atomic<float> lfoPitchModAmount{0.0f};
    std::atomic<float> lfoFilterModAmount{0.0f};
    std::atomic<bool> lfoSyncToTempo{false};
    void clearExtInputs() {
        extVco1Pitch_ = 0.0f;
        extVco2Pitch_ = 0.0f;
        extFilterCv_ = 0.0f;
        extGateIn_ = 0.0f;}
    void addExtVco1Pitch(float semis) { extVco1Pitch_ += semis; }
    void addExtVco2Pitch(float semis) { extVco2Pitch_ += semis; }
    void addExtFilterCv(float hz) { extFilterCv_ += hz; }
    void addExtGateIn(float v) { extGateIn_ += v; }
    float lastVco1() const { return lastVco1_; }
    float lastVco2() const { return lastVco2_; }
    float lastRing() const { return lastRing_; }
    float lastEg() const { return lastEg_; }
    float lastLfo() const { return lastLfo_; }
    float lastFilterOut() const { return lastFilterOut_; }
    float lastGate() const { return lastGate_; }
private:
    Oscillator vco1_;
    Oscillator vco2_;
    FilterSVF hpf_;
    FilterSVF lpf_;
    EnvelopeADSR eg_;
    LFO lfo_;
    VCA vca_;
    std::atomic<int> currentNote_{60};
    std::atomic<int> heldNote_{-1};
    std::atomic<bool> noteOnPending_{false};
    std::atomic<bool> noteOffPending_{false};
    std::atomic<float> externalTempo_{120.0f};
    std::vector<Param> params_;
    float sampleRate_ = 44100.0f;
    float extVco1Pitch_ = 0.0f;
    float extVco2Pitch_ = 0.0f;
    float extFilterCv_ = 0.0f;
    float extGateIn_ = 0.0f;
    bool extGateInPrev_ = false;
    float lastVco1_ = 0.0f;
    float lastVco2_ = 0.0f;
    float lastRing_ = 0.0f;
    float lastEg_ = 0.0f;
    float lastLfo_ = 0.0f;
    float lastFilterOut_ = 0.0f;
    float lastGate_ = 0.0f;
};

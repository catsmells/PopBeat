#include "Ms20Voice.h"
#include "../audio/Pitch.h"
Ms20Voice::Ms20Voice() {
    vco1_.setWaveform(Waveform::Saw);
    vco2_.setWaveform(Waveform::Saw);
    hpf_.setMode(SvfMode::HighPass);
    lpf_.setMode(SvfMode::LowPass);
    lfo_.setWaveform(Waveform::Triangle);
    vca_.setSmoothingMs(2.0f, sampleRate_);
    params_ = {
        {"VCO1 Tune", &vco1Tune, -24.0f, 24.0f, 0.0f},
        {"VCO2 Tune", &vco2Tune, -24.0f, 24.0f, -12.0f},
        {"Mix VCO1", &mixVco1, 0.0f, 1.0f, 0.7f},
        {"Mix VCO2", &mixVco2, 0.0f, 1.0f, 0.3f},
        {"Ring Mod", &mixRingMod, 0.0f, 1.0f, 0.0f},
        {"HPF Cutoff", &hpfCutoff, 20.0f, 4000.0f, 80.0f},
        {"LPF Cutoff", &lpfCutoff, 40.0f, 8000.0f, 2000.0f},
        {"LPF Resonance", &lpfResonance, 0.0f, 1.0f, 0.3f},
        {"EG Attack", &egAttack, 0.5f, 2000.0f, 5.0f},
        {"EG Decay", &egDecay, 5.0f, 2000.0f, 100.0f},
        {"EG Sustain", &egSustain, 0.0f, 1.0f, 0.7f},
        {"EG Release", &egRelease, 5.0f, 4000.0f, 200.0f},
        {"LFO Rate", &lfoRate, 0.1f, 20.0f, 4.0f},
        {"LFO->Pitch", &lfoPitchModAmount, 0.0f, 12.0f, 0.0f},
        {"LFO->Filter", &lfoFilterModAmount, 0.0f, 4000.0f, 0.0f},
    };
}
void Ms20Voice::setSampleRate(float sr) {
    sampleRate_ = sr;
    vco1_.setSampleRate(sr);
    vco2_.setSampleRate(sr);
    hpf_.setSampleRate(sr);
    lpf_.setSampleRate(sr);
    eg_.setSampleRate(sr);
    lfo_.setSampleRate(sr);
    vca_.setSmoothingMs(2.0f, sr);
}
void Ms20Voice::reset() {
    vco1_.reset();
    vco2_.reset();
    hpf_.reset();
    lpf_.reset();
    eg_.reset();
    lfo_.reset();
    vca_.reset();
    clearExtInputs();
    extGateInPrev_ = false;
    lastVco1_ = lastVco2_ = lastRing_ = lastEg_ = lastLfo_ = lastFilterOut_ = lastGate_ = 0.0f;
}
void Ms20Voice::noteOn(int midiNote) {
    currentNote_.store(midiNote, std::memory_order_relaxed);
    heldNote_.store(midiNote, std::memory_order_relaxed);
    noteOnPending_.store(true, std::memory_order_relaxed);
}
void Ms20Voice::noteOff(int midiNote) {
    if (heldNote_.load(std::memory_order_relaxed) == midiNote) {
        heldNote_.store(-1, std::memory_order_relaxed);
        noteOffPending_.store(true, std::memory_order_relaxed);
    }
}
float Ms20Voice::process() {
    if (noteOnPending_.exchange(false, std::memory_order_relaxed)) {
        eg_.noteOn();
    }
    if (noteOffPending_.exchange(false, std::memory_order_relaxed)) {
        eg_.noteOff();
    }
    const bool gateInHigh = extGateIn_ > 0.5f;
    if (gateInHigh && !extGateInPrev_) {
        eg_.noteOn();
    } else if (!gateInHigh && extGateInPrev_) {
        eg_.noteOff();
    }
    extGateInPrev_ = gateInHigh;
    eg_.setAttackMs(egAttack.load(std::memory_order_relaxed));
    eg_.setDecayMs(egDecay.load(std::memory_order_relaxed));
    eg_.setSustain(egSustain.load(std::memory_order_relaxed));
    eg_.setReleaseMs(egRelease.load(std::memory_order_relaxed));
    if (lfoSyncToTempo.load(std::memory_order_relaxed)) {
        lfo_.setRateHz(externalTempo_.load(std::memory_order_relaxed) / 60.0f);  // quarter-note
    } else {
        lfo_.setRateHz(lfoRate.load(std::memory_order_relaxed));
    }
    const float lfoV = lfo_.process();
    const float pitchMod = lfoV * lfoPitchModAmount.load(std::memory_order_relaxed);
    const float baseHz = midiToHz(currentNote_.load(std::memory_order_relaxed));
    const float tune1 = vco1Tune.load(std::memory_order_relaxed);
    const float tune2 = vco2Tune.load(std::memory_order_relaxed);
    vco1_.setFrequency(baseHz * std::pow(2.0f, (tune1 + pitchMod + extVco1Pitch_) / 12.0f));
    vco2_.setFrequency(baseHz * std::pow(2.0f, (tune2 + pitchMod + extVco2Pitch_) / 12.0f));
    bool vco1Wrapped = false;
    const float osc1 = vco1_.process(false, &vco1Wrapped);
    const bool syncVco2 = hardSyncEnabled.load(std::memory_order_relaxed) && vco1Wrapped;
    const float osc2 = vco2_.process(syncVco2);
    const float ringMod = osc1 * osc2;
    const float mixed = osc1 * mixVco1.load(std::memory_order_relaxed) +
                         osc2 * mixVco2.load(std::memory_order_relaxed) +
                         ringMod * mixRingMod.load(std::memory_order_relaxed);
    hpf_.setCutoff(hpfCutoff.load(std::memory_order_relaxed));
    const float hpfOut = hpf_.process(mixed);
    const float filterMod = lfoV * lfoFilterModAmount.load(std::memory_order_relaxed);
    lpf_.setCutoff(lpfCutoff.load(std::memory_order_relaxed) + filterMod);
    lpf_.setResonance(lpfResonance.load(std::memory_order_relaxed));
    const float lpfOut = lpf_.process(hpfOut);
    const float egv = eg_.process();
    return vca_.process(lpfOut, egv);
}

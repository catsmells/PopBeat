#include "DfamVoice.h"
#include "../audio/Pitch.h"
DfamVoice::DfamVoice() {
    vco1_.setWaveform(Waveform::Saw);
    vco2_.setWaveform(Waveform::Square);
    filter_.setMode(SvfMode::LowPass);
    vca_.setSmoothingMs(2.0f, sampleRate_);
    const float defaultSemitones[StepSequencer::kNumSteps] = {0, 0, 7, 0, 12, 0, 7, 3};
    for (int i = 0; i < StepSequencer::kNumSteps; ++i) {
        seq_.setStep(i, Step{defaultSemitones[i], true});
    }
    params_ = {
        {"VCO1 Tune", &vco1Tune, -24.0f, 24.0f, 0.0f},
        {"VCO2 Tune", &vco2Tune, -24.0f, 24.0f, 0.0f},
        {"Mix VCO1", &mixVco1, 0.0f, 1.0f, 0.7f},
        {"Mix VCO2", &mixVco2, 0.0f, 1.0f, 0.0f},
        {"Mix Noise", &mixNoise, 0.0f, 1.0f, 0.0f},
        {"Filter Cutoff", &filterCutoff, 40.0f, 8000.0f, 800.0f},
        {"Filter Resonance", &filterResonance, 0.0f, 1.0f, 0.3f},
        {"EG1 Attack", &eg1Attack, 0.5f, 500.0f, 2.0f},
        {"EG1 Decay", &eg1Decay, 5.0f, 2000.0f, 150.0f},
        {"EG2 Attack", &eg2Attack, 0.5f, 500.0f, 2.0f},
        {"EG2 Decay", &eg2Decay, 5.0f, 2000.0f, 300.0f},
        {"EG2->Pitch", &eg2PitchModAmount, 0.0f, 36.0f, 0.0f},
        {"EG2->Filter", &eg2FilterModAmount, 0.0f, 6000.0f, 0.0f},
    };
}
void DfamVoice::setSampleRate(float sr) {
    sampleRate_ = sr;
    vco1_.setSampleRate(sr);
    vco2_.setSampleRate(sr);
    filter_.setSampleRate(sr);
    eg1_.setSampleRate(sr);
    eg2_.setSampleRate(sr);
    vca_.setSmoothingMs(2.0f, sr);
    seq_.setSampleRate(sr);
}
void DfamVoice::reset() {
    vco1_.reset();
    vco2_.reset();
    filter_.reset();
    eg1_.reset();
    eg2_.reset();
    vca_.reset();
    seq_.reset();
    clearExtInputs();
    extTrigInPrev_ = false;
    lastVco1_ = lastVco2_ = lastNoise_ = lastEg1_ = lastEg2_ = lastFilterOut_ = lastTrig_ = 0.0f;
    trigSamplesRemaining_ = 0;
}
float DfamVoice::process() {
    eg1_.setAttackMs(eg1Attack.load(std::memory_order_relaxed));
    eg1_.setDecayMs(eg1Decay.load(std::memory_order_relaxed));
    eg2_.setAttackMs(eg2Attack.load(std::memory_order_relaxed));
    eg2_.setDecayMs(eg2Decay.load(std::memory_order_relaxed));
    float stepSemitone = 0.0f;
    const bool stepEdge = seq_.advance(stepSemitone);
    const bool trigInHigh = extTrigIn_ > 0.5f;
    const bool trigInEdge = trigInHigh && !extTrigInPrev_;
    extTrigInPrev_ = trigInHigh;
    if (stepEdge || trigInEdge) {
        eg1_.trigger();
        eg2_.trigger();
        trigSamplesRemaining_ = kTrigPulseSamples;
    }
    const float eg1v = eg1_.process();
    const float eg2v = eg2_.process();
    const float pitchMod = eg2v * eg2PitchModAmount.load(std::memory_order_relaxed);
    vco1_.setFrequency(semitoneToHz(
        vco1Tune.load(std::memory_order_relaxed) + stepSemitone + pitchMod + extVco1Pitch_,
        kBaseHz));
    vco2_.setFrequency(semitoneToHz(
        vco2Tune.load(std::memory_order_relaxed) + stepSemitone + pitchMod + extVco2Pitch_,
        kBaseHz));
    const float osc1 = vco1_.process(false);
    const float osc2 = vco2_.process(false);
    const float noiseS = noise_.process();
    const float mixed = osc1 * mixVco1.load(std::memory_order_relaxed) +
                         osc2 * mixVco2.load(std::memory_order_relaxed) +
                         noiseS * mixNoise.load(std::memory_order_relaxed);
    const float filterMod = eg2v * eg2FilterModAmount.load(std::memory_order_relaxed);
    filter_.setCutoff(filterCutoff.load(std::memory_order_relaxed) + filterMod + extFilterCv_);
    filter_.setResonance(filterResonance.load(std::memory_order_relaxed));
    const float filtered = filter_.process(mixed);
    const float out = vca_.process(filtered, eg1v);
    lastVco1_ = osc1;
    lastVco2_ = osc2;
    lastNoise_ = noiseS;
    lastEg1_ = eg1v;
    lastEg2_ = eg2v;
    lastFilterOut_ = filtered;
    if (trigSamplesRemaining_ > 0) {
        lastTrig_ = static_cast<float>(trigSamplesRemaining_) / kTrigPulseSamples;
        --trigSamplesRemaining_;
    } else {
        lastTrig_ = 0.0f;
    }
    return(out);
}

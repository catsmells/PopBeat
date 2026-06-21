#include "DfamPanel.h"
#include <string>
#include "Font.h"
namespace {
constexpr SDL_Color kPanelBg{18, 16, 15, 255};
constexpr SDL_Color kKnobBody{232, 220, 192, 255};
constexpr SDL_Color kAccent{217, 80, 31, 255};
constexpr SDL_Color kAccentDim{90, 45, 25, 255};
constexpr SDL_Color kLedOn{255, 174, 66, 255};
constexpr SDL_Color kLedOff{50, 36, 28, 255};
constexpr SDL_Color kToggleOff{40, 36, 34, 255};
constexpr SDL_Color kText{217, 80, 31, 255};
constexpr int kKnobRadius = 26;
constexpr int kKnobSpacing = 88;
}
DfamPanel::DfamPanel(DfamVoice& voice, SDL_Rect area)
    : voice_(voice), area_(area), originX_(area.x), originY_(area.y) {
    int x = originX_ + 50;
    const int row1Y = originY_ + 60;
    knobs_.emplace_back(x, row1Y, kKnobRadius, &voice_.vco1Tune, -24.0f, 24.0f, "VCO1 TUNE",
                         kKnobBody, kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row1Y, kKnobRadius, &voice_.mixVco1, 0.0f, 1.0f, "VCO1 LVL", kKnobBody,
                         kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row1Y, kKnobRadius, &voice_.vco2Tune, -24.0f, 24.0f, "VCO2 TUNE",
                         kKnobBody, kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row1Y, kKnobRadius, &voice_.mixVco2, 0.0f, 1.0f, "VCO2 LVL", kKnobBody,
                         kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row1Y, kKnobRadius, &voice_.mixNoise, 0.0f, 1.0f, "NOISE LVL",
                         kKnobBody, kAccent);
    x += kKnobSpacing + 30;
    knobs_.emplace_back(x, row1Y, kKnobRadius + 4, &voice_.filterCutoff, 40.0f, 8000.0f, "CUTOFF",
                         kKnobBody, kAccent);
    x += kKnobSpacing + 8;
    knobs_.emplace_back(x, row1Y, kKnobRadius + 4, &voice_.filterResonance, 0.0f, 1.0f, "RESONANCE",
                         kKnobBody, kAccent);
    x = originX_ + 50;
    const int row2Y = row1Y + 110;
    knobs_.emplace_back(x, row2Y, kKnobRadius, &voice_.eg1Attack, 0.5f, 500.0f, "EG1 ATTACK",
                         kKnobBody, kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row2Y, kKnobRadius, &voice_.eg1Decay, 5.0f, 2000.0f, "EG1 DECAY",
                         kKnobBody, kAccent);
    x += kKnobSpacing + 20;
    knobs_.emplace_back(x, row2Y, kKnobRadius, &voice_.eg2Attack, 0.5f, 500.0f, "EG2 ATTACK",
                         kKnobBody, kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row2Y, kKnobRadius, &voice_.eg2Decay, 5.0f, 2000.0f, "EG2 DECAY",
                         kKnobBody, kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row2Y, kKnobRadius, &voice_.eg2PitchModAmount, 0.0f, 36.0f,
                         "EG2:PITCH", kKnobBody, kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row2Y, kKnobRadius, &voice_.eg2FilterModAmount, 0.0f, 6000.0f,
                         "EG2:FILT", kKnobBody, kAccent);
    const int stepSpacing = 110;
    int sx = originX_ + 50;
    const int stepRowTopY = row2Y + 80;
    const int ledY = stepRowTopY + 70;
    const int stepKnobY = ledY + 40;
    const int toggleY = stepKnobY + 50;
    for (int i = 0; i < StepSequencer::kNumSteps; ++i) {
        stepLeds_.emplace_back(sx, ledY, 6, [this, i] { return voice_.sequencer().currentStep() == i; },
                                kLedOn, kLedOff);
        StepSequencer* seq = &voice_.sequencer();
        stepPitchKnobs_.emplace_back(
            sx, stepKnobY, 22, [seq, i] { return seq->getStep(i).semitone; },
            [seq, i](float v) {
                Step s = seq->getStep(i);
                s.semitone = v;
                seq->setStep(i, s);
            },
            -24.0f, 24.0f, "STEP " + std::to_string(i + 1), kKnobBody, kAccent);
        SDL_Rect toggleRect{sx - 14, toggleY, 28, 16};
        stepToggles_.emplace_back(
            toggleRect, [seq, i] { return seq->getStep(i).enabled; },
            [seq, i](bool on) {
                Step s = seq->getStep(i);
                s.enabled = on;
                seq->setStep(i, s);
            },
            "", kAccent, kToggleOff);

        sx += stepSpacing;
    }
    StepSequencer* seq = &voice_.sequencer();
    SDL_Rect runRect{sx + 10, toggleY - 14, 70, 36};
    runStopToggle_.emplace(runRect, [seq] { return seq->isRunning(); },
                            [seq](bool on) { seq->setRunning(on); }, "RUN", kAccent, kToggleOff);
    SDL_Rect tempoRect{originX_ + 50, toggleY + 60, stepSpacing * StepSequencer::kNumSteps, 8};
    tempoSlider_.emplace(tempoRect, [seq] { return seq->getBpm(); },
                          [seq](float v) { seq->setBpm(v); }, 40.0f, 240.0f, "TEMPO", kAccentDim,
                          kAccent);
}
void DfamPanel::handleEvent(const SDL_Event& e) {
    for (auto& k : knobs_) k.handleEvent(e);
    for (auto& k : stepPitchKnobs_) k.handleEvent(e);
    for (auto& t : stepToggles_) t.handleEvent(e);
    if (runStopToggle_) runStopToggle_->handleEvent(e);
    if (tempoSlider_) tempoSlider_->handleEvent(e);
}
void DfamPanel::draw(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, kPanelBg.r, kPanelBg.g, kPanelBg.b, kPanelBg.a);
    SDL_RenderFillRect(renderer, &area_);
    Font::drawText(renderer, originX_ + 30, originY_ + 10, "MOOG DFAM VOICE", kText, 2);
    for (const auto& k : knobs_) k.draw(renderer);
    for (const auto& l : stepLeds_) l.draw(renderer);
    for (const auto& k : stepPitchKnobs_) k.draw(renderer);
    for (const auto& t : stepToggles_) t.draw(renderer);
    if (runStopToggle_) runStopToggle_->draw(renderer);
    if (tempoSlider_) tempoSlider_->draw(renderer);
}

#include "Ms20Panel.h"
#include "Font.h"
namespace {
constexpr SDL_Color kPanelBg{16, 14, 13, 255};
constexpr SDL_Color kKnobBody{216, 210, 196, 255};
constexpr SDL_Color kAccent{201, 122, 43, 255};
constexpr SDL_Color kToggleOff{40, 36, 34, 255};
constexpr SDL_Color kText{201, 122, 43, 255};
constexpr SDL_Color kWhiteKey{226, 220, 208, 255};
constexpr SDL_Color kBlackKey{30, 26, 24, 255};
constexpr SDL_Color kKeyPressed{201, 122, 43, 255};
constexpr int kKnobRadius = 24;
constexpr int kKnobSpacing = 78;
bool isBlackKey(int note) {
    const int pc = ((note % 12) + 12) % 12;
    return pc == 1 || pc == 3 || pc == 6 || pc == 8 || pc == 10;
}}
Ms20Panel::Ms20Panel(Ms20Voice& voice, SDL_Rect area)
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
    knobs_.emplace_back(x, row1Y, kKnobRadius, &voice_.mixRingMod, 0.0f, 1.0f, "RING MOD",
                         kKnobBody, kAccent);
    x += kKnobSpacing + 20;
    SDL_Rect syncRect{x - 18, row1Y - 12, 36, 24};
    hardSyncToggle_.emplace(
        syncRect, [this] { return voice_.hardSyncEnabled.load(std::memory_order_relaxed); },
        [this](bool on) { voice_.hardSyncEnabled.store(on, std::memory_order_relaxed); }, "SYNC",
        kAccent, kToggleOff);
    x = originX_ + 50;
    const int row2Y = row1Y + 100;
    knobs_.emplace_back(x, row2Y, kKnobRadius + 2, &voice_.hpfCutoff, 20.0f, 4000.0f, "HPF CUTOFF",
                         kKnobBody, kAccent);
    x += kKnobSpacing + 10;
    knobs_.emplace_back(x, row2Y, kKnobRadius + 2, &voice_.lpfCutoff, 40.0f, 8000.0f, "LPF CUTOFF",
                         kKnobBody, kAccent);
    x += kKnobSpacing + 10;
    knobs_.emplace_back(x, row2Y, kKnobRadius + 2, &voice_.lpfResonance, 0.0f, 1.0f,
                         "LPF RESONANCE", kKnobBody, kAccent);
    x += kKnobSpacing + 30;
    knobs_.emplace_back(x, row2Y, kKnobRadius, &voice_.lfoRate, 0.1f, 20.0f, "LFO RATE", kKnobBody,
                         kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row2Y, kKnobRadius, &voice_.lfoPitchModAmount, 0.0f, 12.0f,
                         "LFO:PITCH", kKnobBody, kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row2Y, kKnobRadius, &voice_.lfoFilterModAmount, 0.0f, 4000.0f,
                         "LFO:FILT", kKnobBody, kAccent);
    x += kKnobSpacing + 12;
    SDL_Rect lfoSyncRect{x - 18, row2Y - 12, 36, 24};
    lfoSyncToggle_.emplace(
        lfoSyncRect, [this] { return voice_.lfoSyncToTempo.load(std::memory_order_relaxed); },
        [this](bool on) { voice_.lfoSyncToTempo.store(on, std::memory_order_relaxed); },
        "LFO SYNC", kAccent, kToggleOff);
    x = originX_ + 50;
    const int row3Y = row2Y + 100;
    knobs_.emplace_back(x, row3Y, kKnobRadius, &voice_.egAttack, 0.5f, 2000.0f, "EG ATTACK",
                         kKnobBody, kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row3Y, kKnobRadius, &voice_.egDecay, 5.0f, 2000.0f, "EG DECAY",
                         kKnobBody, kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row3Y, kKnobRadius, &voice_.egSustain, 0.0f, 1.0f, "EG SUSTAIN",
                         kKnobBody, kAccent);
    x += kKnobSpacing;
    knobs_.emplace_back(x, row3Y, kKnobRadius, &voice_.egRelease, 5.0f, 4000.0f, "EG RELEASE",
                         kKnobBody, kAccent);
    constexpr int kWhiteW = 30, kWhiteH = 100, kBlackW = 18, kBlackH = 60;
    const int keyboardY = row3Y + 90;
    int wx = originX_ + 50;
    for (int note = 48; note <= 72; ++note) {
        if (!isBlackKey(note)) {
            whiteKeys_.push_back(PianoKey{SDL_Rect{wx, keyboardY, kWhiteW, kWhiteH}, note});
            wx += kWhiteW;
        }
    }
    wx = originX_ + 50;
    for (int note = 48; note <= 72; ++note) {
        if (!isBlackKey(note)) {
            wx += kWhiteW;
        } else {
            blackKeys_.push_back(
                PianoKey{SDL_Rect{wx - kBlackW / 2, keyboardY, kBlackW, kBlackH}, note});
        }
    }
}
int Ms20Panel::noteForScancode(SDL_Scancode sc) const {
    switch (sc) {
        case SDL_SCANCODE_A: return kBaseNote + 0;
        case SDL_SCANCODE_W: return kBaseNote + 1;
        case SDL_SCANCODE_S: return kBaseNote + 2;
        case SDL_SCANCODE_E: return kBaseNote + 3;
        case SDL_SCANCODE_D: return kBaseNote + 4;
        case SDL_SCANCODE_F: return kBaseNote + 5;
        case SDL_SCANCODE_T: return kBaseNote + 6;
        case SDL_SCANCODE_G: return kBaseNote + 7;
        case SDL_SCANCODE_Y: return kBaseNote + 8;
        case SDL_SCANCODE_H: return kBaseNote + 9;
        case SDL_SCANCODE_U: return kBaseNote + 10;
        case SDL_SCANCODE_J: return kBaseNote + 11;
        case SDL_SCANCODE_K: return kBaseNote + 12;
        default: return -1;
    }
}
void Ms20Panel::pressNote(int midiNote) {
    if (mouseHeldNote_ != -1) {
        pressedNotes_[static_cast<size_t>(mouseHeldNote_)] = false;
        voice_.noteOff(mouseHeldNote_);
    }
    mouseHeldNote_ = midiNote;
    pressedNotes_[static_cast<size_t>(midiNote)] = true;
    voice_.noteOn(midiNote);
}
void Ms20Panel::releaseNote(int midiNote) {
    if (mouseHeldNote_ == midiNote) {
        pressedNotes_[static_cast<size_t>(midiNote)] = false;
        voice_.noteOff(midiNote);
        mouseHeldNote_ = -1;
    }
}
void Ms20Panel::handleEvent(const SDL_Event& e) {
    for (auto& k : knobs_) k.handleEvent(e);
    if (hardSyncToggle_) hardSyncToggle_->handleEvent(e);
    if (lfoSyncToggle_) lfoSyncToggle_->handleEvent(e);
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        const int mx = e.button.x, my = e.button.y;
        for (const auto& k : blackKeys_) {
            if (mx >= k.rect.x && mx < k.rect.x + k.rect.w && my >= k.rect.y &&
                my < k.rect.y + k.rect.h) {
                pressNote(k.midiNote);
                return;
            }
        }
        for (const auto& k : whiteKeys_) {
            if (mx >= k.rect.x && mx < k.rect.x + k.rect.w && my >= k.rect.y &&
                my < k.rect.y + k.rect.h) {
                pressNote(k.midiNote);
                return;
            }
        }
    } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        if (mouseHeldNote_ != -1) releaseNote(mouseHeldNote_);
    }
}
void Ms20Panel::handleKeyboardNote(const SDL_Event& e) {
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        const int note = noteForScancode(e.key.keysym.scancode);
        if (note >= 0) {
            pressedNotes_[static_cast<size_t>(note)] = true;
            voice_.noteOn(note);
        }
    } else if (e.type == SDL_KEYUP) {
        const int note = noteForScancode(e.key.keysym.scancode);
        if (note >= 0) {
            pressedNotes_[static_cast<size_t>(note)] = false;
            voice_.noteOff(note);
        }
    }
}
void Ms20Panel::draw(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, kPanelBg.r, kPanelBg.g, kPanelBg.b, kPanelBg.a);
    SDL_RenderFillRect(renderer, &area_);
    Font::drawText(renderer, originX_ + 30, originY_ + 10, "KORG MS-20 VOICE", kText, 2);
    for (const auto& k : knobs_) k.draw(renderer);
    if (hardSyncToggle_) hardSyncToggle_->draw(renderer);
    if (lfoSyncToggle_) lfoSyncToggle_->draw(renderer);
    for (const auto& k : whiteKeys_) {
        const bool pressed = pressedNotes_[static_cast<size_t>(k.midiNote)];
        SDL_Color c = pressed ? kKeyPressed : kWhiteKey;
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(renderer, &k.rect);
        SDL_SetRenderDrawColor(renderer, kBlackKey.r, kBlackKey.g, kBlackKey.b, 255);
        SDL_RenderDrawRect(renderer, &k.rect);
    }
    for (const auto& k : blackKeys_) {
        const bool pressed = pressedNotes_[static_cast<size_t>(k.midiNote)];
        SDL_Color c = pressed ? kKeyPressed : kBlackKey;
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(renderer, &k.rect);
    }
}

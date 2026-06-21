#pragma once
#include <SDL2/SDL.h>
#include <array>
#include <optional>
#include <vector>
#include "../voices/Ms20Voice.h"
#include "Widgets.h"
struct PianoKey {
    SDL_Rect rect;
    int midiNote;
};
class Ms20Panel {
public:
    Ms20Panel(Ms20Voice& voice, SDL_Rect area);
    void handleEvent(const SDL_Event& e);
    void handleKeyboardNote(const SDL_Event& e);
    void draw(SDL_Renderer* renderer) const;
private:
    void pressNote(int midiNote);
    void releaseNote(int midiNote);
    int noteForScancode(SDL_Scancode sc) const;
    Ms20Voice& voice_;
    SDL_Rect area_;
    int originX_, originY_;
    std::vector<Knob> knobs_;
    std::optional<Toggle> hardSyncToggle_;
    std::optional<Toggle> lfoSyncToggle_;
    std::vector<PianoKey> whiteKeys_;
    std::vector<PianoKey> blackKeys_;
    std::array<bool, 128> pressedNotes_{};
    int mouseHeldNote_ = -1;
    static constexpr int kBaseNote = 60;
};

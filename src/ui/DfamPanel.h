#pragma once
#include <SDL2/SDL.h>
#include <optional>
#include <vector>
#include "../voices/DfamVoice.h"
#include "Widgets.h"
class DfamPanel {
public:
    DfamPanel(DfamVoice& voice, SDL_Rect area);
    void handleEvent(const SDL_Event& e);
    void draw(SDL_Renderer* renderer) const;
private:
    DfamVoice& voice_;
    SDL_Rect area_;
    int originX_, originY_;
    std::vector<Knob> knobs_;
    std::vector<Knob> stepPitchKnobs_;
    std::vector<Toggle> stepToggles_;
    std::vector<LED> stepLeds_;
    std::optional<Toggle> runStopToggle_;
    std::optional<Slider> tempoSlider_;
};

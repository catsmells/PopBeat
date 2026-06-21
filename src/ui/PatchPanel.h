#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "../audio/PatchBay.h"
#include "Widgets.h"
class PatchPanel {
public:
    PatchPanel(PatchBay& patchBay, SDL_Rect area);
    void handleEvent(const SDL_Event& e);
    void draw(SDL_Renderer* renderer) const;
private:
    const PatchJack* jackAt(int px, int py) const;
    bool isConnected(JackId id) const;
    void handleJackClick(JackId id);
    PatchBay& patchBay_;
    SDL_Rect area_;
    std::vector<PatchJack> jacks_;
    bool dragging_ = false;
    JackId dragOrigin_ = JackId::DfamVco1;
    int mouseX_ = 0;
    int mouseY_ = 0;
};

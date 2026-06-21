#pragma once
#include <SDL2/SDL.h>
#include <optional>
#include <string>
#include <vector>
#include "../audio/Engine.h"
#include "../audio/Presets.h"
#include "DfamPanel.h"
#include "Ms20Panel.h"
#include "PatchPanel.h"
#include "Widgets.h"
enum class ActivePanel { Dfam, Ms20, Patch };
class App {
public:
    App(SDL_Renderer* renderer, Engine& engine, int windowW, int windowH);
    void handleEvent(const SDL_Event& e);
    void render(SDL_Renderer* renderer);
private:
    void onExportClicked();
    Engine& engine_;
    ActivePanel active_ = ActivePanel::Dfam;
    int windowW_;
    int windowH_;
    SDL_Rect dfamTabRect_;
    SDL_Rect ms20TabRect_;
    SDL_Rect patchTabRect_;
    DfamPanel dfamPanel_;
    Ms20Panel ms20Panel_;
    PatchPanel patchPanel_;
    std::optional<Slider> masterVolumeSlider_;
    std::optional<Toggle> recordToggle_;
    std::optional<Button> exportButton_;
    std::vector<Button> presetButtons_;
    std::string statusMessage_;
    Uint32 statusExpiresAtMs_ = 0;
};

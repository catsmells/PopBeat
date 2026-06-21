#include "App.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "Font.h"
namespace {
constexpr SDL_Color kBgColor{8, 7, 7, 255};
constexpr SDL_Color kTabActive{217, 80, 31, 255};
constexpr SDL_Color kTabInactive{50, 44, 40, 255};
constexpr SDL_Color kTabText{230, 220, 205, 255};
constexpr SDL_Color kMasterAccent{230, 220, 205, 255};
constexpr SDL_Color kMasterTrack{40, 36, 34, 255};
constexpr SDL_Color kRecOn{217, 40, 30, 255};
constexpr SDL_Color kRecOff{50, 44, 40, 255};
constexpr SDL_Color kButtonBody{50, 44, 40, 255};
constexpr SDL_Color kButtonText{230, 220, 205, 255};
constexpr SDL_Color kStatusText{180, 170, 155, 255};
constexpr SDL_Color kPresetBarBg{14, 13, 12, 255};
constexpr SDL_Color kPresetButton{50, 44, 40, 255};
constexpr SDL_Color kPresetText{230, 220, 205, 255};
constexpr SDL_Color kPresetLabel{140, 132, 122, 255};
constexpr int kTabBarHeight = 72;
constexpr int kContentY = kTabBarHeight + 10;
constexpr int kPresetBarHeight = 54;
}
App::App(SDL_Renderer*, Engine& engine, int windowW, int windowH)
    : engine_(engine),
      windowW_(windowW),
      windowH_(windowH),
      dfamTabRect_{20, 10, 120, 36},
      ms20TabRect_{148, 10, 120, 36},
      patchTabRect_{276, 10, 120, 36},
      dfamPanel_(engine.dfam(),
                 SDL_Rect{0, kContentY, windowW, windowH - kContentY - kPresetBarHeight}),
      ms20Panel_(engine.ms20(),
                 SDL_Rect{0, kContentY, windowW, windowH - kContentY - kPresetBarHeight}),
      patchPanel_(engine.patchBay(),
                  SDL_Rect{0, kContentY, windowW, windowH - kContentY - kPresetBarHeight}) {
    SDL_Rect volRect{windowW - 220, 22, 160, 8};
    masterVolumeSlider_.emplace(volRect, &engine_.masterVolume, 0.0f, 1.0f, "MASTER",
                                 kMasterTrack, kMasterAccent);
    SDL_Rect recRect{410, 10, 70, 36};
    recordToggle_.emplace(
        recRect, [this] { return engine_.recorder().isRecording(); },
        [this](bool on) {
            if (on) {
                engine_.recorder().beginRecording();
            } else {
                engine_.recorder().stopRecording();
            }
        },
        "REC", kRecOn, kRecOff);
    SDL_Rect exportRect{560, 10, 130, 36};
    exportButton_.emplace(exportRect, [this] { onExportClicked(); }, "EXPORT FLAC", kButtonBody,
                          kButtonText);
    const std::vector<Preset>& presets = factoryPresets();
    const int n = static_cast<int>(presets.size());
    constexpr int kMargin = 30;
    constexpr int kGap = 16;
    constexpr int kButtonH = 34;
    const int buttonW = n > 0 ? (windowW - 2 * kMargin - (n - 1) * kGap) / n : 0;
    const int barY = windowH - kPresetBarHeight;
    const int buttonY = barY + (kPresetBarHeight - kButtonH) / 2;
    for (int i = 0; i < n; ++i) {
        const Preset* preset = &presets[static_cast<size_t>(i)];
        SDL_Rect rect{kMargin + i * (buttonW + kGap), buttonY, buttonW, kButtonH};
        presetButtons_.emplace_back(
            rect,
            [this, preset] {
                applyPreset(*preset, engine_.dfam(), engine_.ms20(), engine_.patchBay());
                statusMessage_ = std::string("LOADED: ") + preset->displayName;
                statusExpiresAtMs_ = SDL_GetTicks() + 3000;
            },
            preset->displayName, kPresetButton, kPresetText);
    }
}
void App::onExportClicked() {
    if (engine_.recorder().isRecording()) {
        statusMessage_ = "STOP RECORDING FIRST";
        statusExpiresAtMs_ = SDL_GetTicks() + 3000;
        return;
    }
    const char* home = std::getenv("HOME");
    const std::string dir =
        (home ? std::string(home) : std::string(".")) + "/Music/PopBeat/recordings";
    const std::time_t t = std::time(nullptr);
    char timestamp[32];
    std::strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", std::localtime(&t));
    const std::string path = dir + "/popbeat_" + timestamp + ".flac";
    const bool ok = engine_.recorder().exportToFlac(path);
    statusMessage_ = ok ? ("SAVED: " + path) : std::string("EXPORT FAILED (NOTHING RECORDED?)");
    statusExpiresAtMs_ = SDL_GetTicks() + 5000;
}
void App::handleEvent(const SDL_Event& e) {
    if (masterVolumeSlider_) masterVolumeSlider_->handleEvent(e);
    if (recordToggle_) recordToggle_->handleEvent(e);
    if (exportButton_) exportButton_->handleEvent(e);
    for (auto& b : presetButtons_) b.handleEvent(e);
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        const int mx = e.button.x, my = e.button.y;
        auto inRect = [&](const SDL_Rect& r) {
            return mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h;
        };
        if (inRect(dfamTabRect_)) {
            active_ = ActivePanel::Dfam;
            return;
        }
        if (inRect(ms20TabRect_)) {
            active_ = ActivePanel::Ms20;
            return;
        }
        if (inRect(patchTabRect_)) {
            active_ = ActivePanel::Patch;
            return;
        }
    }
    if (e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP ||
        e.type == SDL_MOUSEMOTION) {
        if (active_ == ActivePanel::Dfam) {
            dfamPanel_.handleEvent(e);
        } else if (active_ == ActivePanel::Ms20) {
            ms20Panel_.handleEvent(e);
        } else {
            patchPanel_.handleEvent(e);
        }
    } else if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
        if (active_ == ActivePanel::Ms20) {
            ms20Panel_.handleKeyboardNote(e);
        }
    }
}
void App::render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, kBgColor.r, kBgColor.g, kBgColor.b, kBgColor.a);
    SDL_RenderClear(renderer);
    auto drawTab = [&](const SDL_Rect& rect, const char* label, bool isActive) {
        const SDL_Color c = isActive ? kTabActive : kTabInactive;
        SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
        SDL_RenderFillRect(renderer, &rect);
        const int textW = Font::textWidth(label, 2);
        Font::drawText(renderer, rect.x + rect.w / 2 - textW / 2, rect.y + rect.h / 2 - 7, label,
                        kTabText, 2);
    };
    drawTab(dfamTabRect_, "DFAM", active_ == ActivePanel::Dfam);
    drawTab(ms20TabRect_, "MS-20", active_ == ActivePanel::Ms20);
    drawTab(patchTabRect_, "PATCH", active_ == ActivePanel::Patch);
    if (active_ == ActivePanel::Dfam) {
        dfamPanel_.draw(renderer);
    } else if (active_ == ActivePanel::Ms20) {
        ms20Panel_.draw(renderer);
    } else {
        patchPanel_.draw(renderer);
    }
    if (masterVolumeSlider_) masterVolumeSlider_->draw(renderer);
    if (recordToggle_) recordToggle_->draw(renderer);
    if (exportButton_) exportButton_->draw(renderer);
    const int barY = windowH_ - kPresetBarHeight;
    SDL_Rect presetBarRect{0, barY, windowW_, kPresetBarHeight};
    SDL_SetRenderDrawColor(renderer, kPresetBarBg.r, kPresetBarBg.g, kPresetBarBg.b,
                           kPresetBarBg.a);
    SDL_RenderFillRect(renderer, &presetBarRect);
    Font::drawText(renderer, 30, barY - 16, "PRESETS", kPresetLabel, 1);
    for (const auto& b : presetButtons_) b.draw(renderer);
    if (engine_.recorder().isRecording()) {
        const int totalSeconds = static_cast<int>(engine_.recorder().elapsedSeconds());
        char timeBuf[16];
        std::snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", totalSeconds / 60, totalSeconds % 60);
        Font::drawText(renderer, 490, 22, timeBuf, kRecOn, 2);
    }
    if (!statusMessage_.empty()) {
        if (SDL_GetTicks() < statusExpiresAtMs_) {
            Font::drawText(renderer, 20, 52, statusMessage_.c_str(), kStatusText, 1);
        } else {
            statusMessage_.clear();
        }
    }
}

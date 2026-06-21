#include "PatchPanel.h"
#include "Font.h"
namespace {
constexpr SDL_Color kPanelBg{14, 13, 12, 255};
constexpr SDL_Color kDfamAccent{217, 80, 31, 255};
constexpr SDL_Color kMs20Accent{201, 122, 43, 255};
constexpr SDL_Color kHeaderText{200, 192, 180, 255};
constexpr SDL_Color kCableColor{230, 220, 205, 255};
constexpr SDL_Color kDragCableColor{255, 255, 255, 255};
constexpr int kJackRadius = 11;
constexpr int kRowSpacing = 56;
constexpr int kRowStartOffsetY = 110;
struct ColumnX {
    int dfamOut, dfamIn, ms20Out, ms20In;
};
ColumnX columnsFor(const SDL_Rect& area) {
    return {area.x + static_cast<int>(area.w * 0.12f), area.x + static_cast<int>(area.w * 0.33f),
            area.x + static_cast<int>(area.w * 0.62f), area.x + static_cast<int>(area.w * 0.83f)};
}
bool pointInCircle(int px, int py, int cx, int cy, int radius) {
    const int dx = px - cx;
    const int dy = py - cy;
    return dx * dx + dy * dy <= radius * radius;
}}
PatchPanel::PatchPanel(PatchBay& patchBay, SDL_Rect area) : patchBay_(patchBay), area_(area) {
    const ColumnX cols = columnsFor(area_);
    const int top = area_.y + kRowStartOffsetY;
    struct Entry {
        JackId id;
        const char* label;
    };
    const Entry dfamOut[] = {{JackId::DfamVco1, "VCO1"},   {JackId::DfamVco2, "VCO2"},
                              {JackId::DfamNoise, "NOISE"}, {JackId::DfamEg1, "EG1"},
                              {JackId::DfamEg2, "EG2"},     {JackId::DfamFilterOut, "FILT OUT"},
                              {JackId::DfamTrig, "TRIG"}};
    const Entry dfamIn[] = {{JackId::DfamVco1Pitch, "VCO1 CV"},
                             {JackId::DfamVco2Pitch, "VCO2 CV"},
                             {JackId::DfamFilterCv, "FILT CV"},
                             {JackId::DfamTrigIn, "TRIG IN"}};
    const Entry ms20Out[] = {{JackId::Ms20Vco1, "VCO1"},   {JackId::Ms20Vco2, "VCO2"},
                              {JackId::Ms20Ring, "RING"},   {JackId::Ms20Eg, "EG"},
                              {JackId::Ms20Lfo, "LFO"},     {JackId::Ms20FilterOut, "FILT OUT"},
                              {JackId::Ms20Gate, "GATE"}};
    const Entry ms20In[] = {{JackId::Ms20Vco1Pitch, "VCO1 CV"},
                             {JackId::Ms20Vco2Pitch, "VCO2 CV"},
                             {JackId::Ms20FilterCv, "FILT CV"},
                             {JackId::Ms20GateIn, "GATE IN"}};
    auto addColumn = [&](int x, const Entry* entries, int count, SDL_Color color) {
        for (int i = 0; i < count; ++i) {
            jacks_.emplace_back(x, top + i * kRowSpacing, kJackRadius, entries[i].id,
                                 entries[i].label, color);
        }
    };
    addColumn(cols.dfamOut, dfamOut, 7, kDfamAccent);
    addColumn(cols.dfamIn, dfamIn, 4, kDfamAccent);
    addColumn(cols.ms20Out, ms20Out, 7, kMs20Accent);
    addColumn(cols.ms20In, ms20In, 4, kMs20Accent);
}
const PatchJack* PatchPanel::jackAt(int px, int py) const {
    for (const auto& j : jacks_) {
        if (pointInCircle(px, py, j.cx(), j.cy(), kJackRadius + 4)) return &j;
    }
    return nullptr;
}
bool PatchPanel::isConnected(JackId id) const {
    std::array<PatchBay::CableView, PatchBay::kMaxCables> conns{};
    const int n = patchBay_.getConnections(conns);
    for (int i = 0; i < n; ++i) {
        if (conns[static_cast<size_t>(i)].source == id || conns[static_cast<size_t>(i)].dest == id)
            return true;
    }
    return false;
}
void PatchPanel::handleJackClick(JackId id) {
    if (!dragging_) {
        if (isConnected(id)) {
            patchBay_.disconnectMostRecent(id);
        } else {
            dragging_ = true;
            dragOrigin_ = id;
        }
        return;
    }
    if (id == dragOrigin_) {
        dragging_ = false;
        return;
    }
    if (isSource(id) == isSource(dragOrigin_)) {
        dragOrigin_ = id;
        return;
    }
    const JackId source = isSource(dragOrigin_) ? dragOrigin_ : id;
    const JackId dest = isSource(dragOrigin_) ? id : dragOrigin_;
    patchBay_.connect(source, dest);
    dragging_ = false;
}
void PatchPanel::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEMOTION) {
        mouseX_ = e.motion.x;
        mouseY_ = e.motion.y;
        return;
    }
    if (e.type != SDL_MOUSEBUTTONDOWN || e.button.button != SDL_BUTTON_LEFT) return;
    const PatchJack* hit = jackAt(e.button.x, e.button.y);
    if (!hit) {
        dragging_ = false;
        return;
    }
    handleJackClick(hit->id());
}
void PatchPanel::draw(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, kPanelBg.r, kPanelBg.g, kPanelBg.b, kPanelBg.a);
    SDL_RenderFillRect(renderer, &area_);
    Font::drawText(renderer, area_.x + 30, area_.y + 10, "PATCH BAY", kHeaderText, 2);
    const ColumnX cols = columnsFor(area_);
    const int headerY = area_.y + kRowStartOffsetY - 36;
    auto header = [&](int x, const char* label) {
        const int w = Font::textWidth(label, 1);
        Font::drawText(renderer, x - w / 2, headerY, label, kHeaderText, 1);
    };
    header(cols.dfamOut, "DFAM OUT");
    header(cols.dfamIn, "DFAM IN");
    header(cols.ms20Out, "MS-20 OUT");
    header(cols.ms20In, "MS-20 IN");
    std::array<PatchBay::CableView, PatchBay::kMaxCables> conns{};
    const int n = patchBay_.getConnections(conns);
    auto findJack = [&](JackId id) -> const PatchJack* {
        for (const auto& j : jacks_) {
            if (j.id() == id) return &j;
        }
        return nullptr;
    };
    SDL_SetRenderDrawColor(renderer, kCableColor.r, kCableColor.g, kCableColor.b, kCableColor.a);
    for (int i = 0; i < n; ++i) {
        const PatchJack* src = findJack(conns[static_cast<size_t>(i)].source);
        const PatchJack* dst = findJack(conns[static_cast<size_t>(i)].dest);
        if (!src || !dst) continue;
        SDL_RenderDrawLine(renderer, src->cx(), src->cy(), dst->cx(), dst->cy());
        SDL_RenderDrawLine(renderer, src->cx(), src->cy() + 1, dst->cx(), dst->cy() + 1);
    }
    if (dragging_) {
        const PatchJack* origin = findJack(dragOrigin_);
        if (origin) {
            SDL_SetRenderDrawColor(renderer, kDragCableColor.r, kDragCableColor.g,
                                   kDragCableColor.b, kDragCableColor.a);
            SDL_RenderDrawLine(renderer, origin->cx(), origin->cy(), mouseX_, mouseY_);
        }
    }
    for (const auto& j : jacks_) {
        j.draw(renderer, isConnected(j.id()), dragging_ && j.id() == dragOrigin_);
    }
}

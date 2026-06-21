#include "Widgets.h"
#include <algorithm>
#include <cmath>
#include "Font.h"
namespace {
constexpr float kPi = 3.14159265358979f;
constexpr float kDegToRad = kPi / 180.0f;
SDL_Color shade(SDL_Color c, float factor) {
    auto clampByte = [](float v) { return static_cast<Uint8>(std::clamp(v, 0.0f, 255.0f)); };
    return SDL_Color{clampByte(c.r * factor), clampByte(c.g * factor), clampByte(c.b * factor),
                      c.a};
}
bool pointInCircle(int px, int py, int cx, int cy, int radius) {
    const int dx = px - cx;
    const int dy = py - cy;
    return dx * dx + dy * dy <= radius * radius;
}
bool pointInRect(int px, int py, const SDL_Rect& r) {
    return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}
}
void drawCircleOutline(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    constexpr int kSegments = 48;
    int prevX = cx + radius;
    int prevY = cy;
    for (int i = 1; i <= kSegments; ++i) {
        const float angle = (static_cast<float>(i) / kSegments) * 2.0f * kPi;
        const int x = cx + static_cast<int>(std::cos(angle) * radius);
        const int y = cy + static_cast<int>(std::sin(angle) * radius);
        SDL_RenderDrawLine(renderer, prevX, prevY, x, y);
        prevX = x;
        prevY = y;
    }
}
void drawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int dy = -radius; dy <= radius; ++dy) {
        const int dx = static_cast<int>(std::sqrt(static_cast<float>(radius * radius - dy * dy)));
        SDL_RenderDrawLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}
void drawArc(SDL_Renderer* renderer, int cx, int cy, int radius, float startDeg, float endDeg,
             SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    constexpr int kSegments = 32;
    const float span = endDeg - startDeg;
    int prevX = cx + static_cast<int>(std::cos(startDeg * kDegToRad) * radius);
    int prevY = cy + static_cast<int>(std::sin(startDeg * kDegToRad) * radius);
    for (int i = 1; i <= kSegments; ++i) {
        const float deg = startDeg + span * (static_cast<float>(i) / kSegments);
        const int x = cx + static_cast<int>(std::cos(deg * kDegToRad) * radius);
        const int y = cy + static_cast<int>(std::sin(deg * kDegToRad) * radius);
        SDL_RenderDrawLine(renderer, prevX, prevY, x, y);
        prevX = x;
        prevY = y;
    }
}
namespace {
constexpr float kKnobMinDeg = 135.0f;
constexpr float kKnobMaxDeg = 405.0f;
}
Knob::Knob(int cx, int cy, int radius, std::atomic<float>* target, float minV, float maxV,
           std::string label, SDL_Color bodyColor, SDL_Color accentColor)
    : Knob(cx, cy, radius, [target] { return target->load(std::memory_order_relaxed); },
           [target](float v) { target->store(v, std::memory_order_relaxed); }, minV, maxV,
           std::move(label), bodyColor, accentColor) {}
Knob::Knob(int cx, int cy, int radius, std::function<float()> getter,
           std::function<void(float)> setter, float minV, float maxV, std::string label,
           SDL_Color bodyColor, SDL_Color accentColor)
    : cx_(cx),
      cy_(cy),
      radius_(radius),
      getter_(std::move(getter)),
      setter_(std::move(setter)),
      minV_(minV),
      maxV_(maxV),
      label_(std::move(label)),
      bodyColor_(bodyColor),
      accentColor_(accentColor) {}
float Knob::value01() const {
    const float v = getter_();
    return std::clamp((v - minV_) / (maxV_ - minV_), 0.0f, 1.0f);
}
void Knob::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        if (pointInCircle(e.button.x, e.button.y, cx_, cy_, radius_)) {
            dragging_ = true;
            dragStartY_ = e.button.y;
            dragStartVal01_ = value01();
        }
    } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        dragging_ = false;
    } else if (e.type == SDL_MOUSEMOTION && dragging_) {
        constexpr float kPixelsForFullRange = 150.0f;
        const float delta = static_cast<float>(dragStartY_ - e.motion.y) / kPixelsForFullRange;
        const float newVal01 = std::clamp(dragStartVal01_ + delta, 0.0f, 1.0f);
        setter_(minV_ + newVal01 * (maxV_ - minV_));
    }
}
void Knob::draw(SDL_Renderer* renderer) const {
    const float val01 = value01();
    const float angleDeg = kKnobMinDeg + val01 * (kKnobMaxDeg - kKnobMinDeg);
    drawArc(renderer, cx_, cy_, radius_ + 5, kKnobMinDeg, kKnobMaxDeg, shade(bodyColor_, 0.6f));
    drawArc(renderer, cx_, cy_, radius_ + 5, kKnobMinDeg, angleDeg, accentColor_);
    drawFilledCircle(renderer, cx_, cy_, radius_, bodyColor_);
    drawCircleOutline(renderer, cx_, cy_, radius_, shade(bodyColor_, 0.5f));
    const float rad = angleDeg * kDegToRad;
    const int px = cx_ + static_cast<int>(std::cos(rad) * radius_ * 0.8f);
    const int py = cy_ + static_cast<int>(std::sin(rad) * radius_ * 0.8f);
    SDL_SetRenderDrawColor(renderer, shade(bodyColor_, 0.3f).r, shade(bodyColor_, 0.3f).g,
                           shade(bodyColor_, 0.3f).b, 255);
    SDL_RenderDrawLine(renderer, cx_, cy_, px, py);
    SDL_RenderDrawLine(renderer, cx_ + 1, cy_, px + 1, py);
    const int textW = Font::textWidth(label_.c_str(), 1);
    Font::drawText(renderer, cx_ - textW / 2, cy_ + radius_ + 8, label_.c_str(), accentColor_, 1);
}
Toggle::Toggle(SDL_Rect rect, std::function<bool()> getter, std::function<void(bool)> setter,
               std::string label, SDL_Color onColor, SDL_Color offColor)
    : rect_(rect),
      getter_(std::move(getter)),
      setter_(std::move(setter)),
      label_(std::move(label)),
      onColor_(onColor),
      offColor_(offColor) {}
void Toggle::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        if (pointInRect(e.button.x, e.button.y, rect_)) {
            setter_(!getter_());
        }
    }
}
void Toggle::draw(SDL_Renderer* renderer) const {
    const bool on = getter_();
    const SDL_Color c = on ? onColor_ : offColor_;
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(renderer, &rect_);
    SDL_SetRenderDrawColor(renderer, shade(c, 0.5f).r, shade(c, 0.5f).g, shade(c, 0.5f).b, 255);
    SDL_RenderDrawRect(renderer, &rect_);
    if (!label_.empty()) {
        const int textW = Font::textWidth(label_.c_str(), 1);
        Font::drawText(renderer, rect_.x + rect_.w / 2 - textW / 2, rect_.y + rect_.h + 4,
                        label_.c_str(), onColor_, 1);
    }
}
LED::LED(int cx, int cy, int radius, std::function<bool()> litGetter, SDL_Color onColor,
         SDL_Color offColor)
    : cx_(cx), cy_(cy), radius_(radius), litGetter_(std::move(litGetter)), onColor_(onColor), offColor_(offColor) {}
void LED::draw(SDL_Renderer* renderer) const {
    const bool lit = litGetter_();
    if (lit) {
        drawFilledCircle(renderer, cx_, cy_, radius_ + 3, shade(onColor_, 0.5f));
    }
    drawFilledCircle(renderer, cx_, cy_, radius_, lit ? onColor_ : offColor_);
}
Slider::Slider(SDL_Rect rect, std::atomic<float>* target, float minV, float maxV,
               std::string label, SDL_Color trackColor, SDL_Color handleColor)
    : Slider(rect, [target] { return target->load(std::memory_order_relaxed); },
             [target](float v) { target->store(v, std::memory_order_relaxed); }, minV, maxV,
             std::move(label), trackColor, handleColor) {}
Slider::Slider(SDL_Rect rect, std::function<float()> getter, std::function<void(float)> setter,
               float minV, float maxV, std::string label, SDL_Color trackColor,
               SDL_Color handleColor)
    : rect_(rect),
      getter_(std::move(getter)),
      setter_(std::move(setter)),
      minV_(minV),
      maxV_(maxV),
      label_(std::move(label)),
      trackColor_(trackColor),
      handleColor_(handleColor) {}
float Slider::value01() const {
    const float v = getter_();
    return std::clamp((v - minV_) / (maxV_ - minV_), 0.0f, 1.0f);
}
void Slider::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT &&
        pointInRect(e.button.x, e.button.y, rect_)) {
        dragging_ = true;
    } else if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        dragging_ = false;
    }
    if (dragging_ && e.type == SDL_MOUSEMOTION) {
        const float val01 = std::clamp(static_cast<float>(e.motion.x - rect_.x) /
                                            static_cast<float>(rect_.w),
                                        0.0f, 1.0f);
        setter_(minV_ + val01 * (maxV_ - minV_));
    }
}
void Slider::draw(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, trackColor_.r, trackColor_.g, trackColor_.b, trackColor_.a);
    SDL_RenderFillRect(renderer, &rect_);
    SDL_SetRenderDrawColor(renderer, shade(trackColor_, 0.5f).r, shade(trackColor_, 0.5f).g,
                           shade(trackColor_, 0.5f).b, 255);
    SDL_RenderDrawRect(renderer, &rect_);
    const int handleX = rect_.x + static_cast<int>(value01() * rect_.w);
    SDL_Rect handle{handleX - 3, rect_.y - 3, 6, rect_.h + 6};
    SDL_SetRenderDrawColor(renderer, handleColor_.r, handleColor_.g, handleColor_.b, handleColor_.a);
    SDL_RenderFillRect(renderer, &handle);
    if (!label_.empty()) {
        Font::drawText(renderer, rect_.x, rect_.y - 14, label_.c_str(), handleColor_, 1);
    }
}
Button::Button(SDL_Rect rect, std::function<void()> onClick, std::string label,
               SDL_Color bodyColor, SDL_Color textColor)
    : rect_(rect),
      onClick_(std::move(onClick)),
      label_(std::move(label)),
      bodyColor_(bodyColor),
      textColor_(textColor) {}
void Button::handleEvent(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT &&
        pointInRect(e.button.x, e.button.y, rect_)) {
        onClick_();
    }
}
void Button::draw(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, bodyColor_.r, bodyColor_.g, bodyColor_.b, bodyColor_.a);
    SDL_RenderFillRect(renderer, &rect_);
    SDL_SetRenderDrawColor(renderer, shade(bodyColor_, 0.5f).r, shade(bodyColor_, 0.5f).g,
                           shade(bodyColor_, 0.5f).b, 255);
    SDL_RenderDrawRect(renderer, &rect_);
    const int textW = Font::textWidth(label_.c_str(), 1);
    Font::drawText(renderer, rect_.x + rect_.w / 2 - textW / 2, rect_.y + rect_.h / 2 - 3,
                    label_.c_str(), textColor_, 1);}
PatchJack::PatchJack(int cx, int cy, int radius, JackId id, std::string label, SDL_Color color)
    : cx_(cx), cy_(cy), radius_(radius), id_(id), label_(std::move(label)), color_(color) {}
bool PatchJack::hitTest(int px, int py) const { return pointInCircle(px, py, cx_, cy_, radius_); }
void PatchJack::draw(SDL_Renderer* renderer, bool connected, bool isDragOrigin) const {
    const SDL_Color holeColor = (connected || isDragOrigin) ? color_ : shade(color_, 0.2f);
    drawFilledCircle(renderer, cx_, cy_, radius_, shade(color_, 0.4f));
    drawCircleOutline(renderer, cx_, cy_, radius_, isDragOrigin ? SDL_Color{255, 255, 255, 255} : color_);
    drawFilledCircle(renderer, cx_, cy_, radius_ / 2, holeColor);
    if (!label_.empty()) {
        const int textW = Font::textWidth(label_.c_str(), 1);
        Font::drawText(renderer, cx_ - textW / 2, cy_ + radius_ + 4, label_.c_str(), color_, 1);
    }
}

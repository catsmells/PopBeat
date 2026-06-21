#pragma once
#include <SDL2/SDL.h>
#include <atomic>
#include <functional>
#include <string>
#include "../audio/PatchBay.h"
void drawCircleOutline(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color);
void drawFilledCircle(SDL_Renderer* renderer, int cx, int cy, int radius, SDL_Color color);
void drawArc(SDL_Renderer* renderer, int cx, int cy, int radius, float startDeg, float endDeg,
             SDL_Color color);
class Knob {
public:
    Knob(int cx, int cy, int radius, std::atomic<float>* target, float minV, float maxV,
         std::string label, SDL_Color bodyColor, SDL_Color accentColor);
    Knob(int cx, int cy, int radius, std::function<float()> getter,
         std::function<void(float)> setter, float minV, float maxV, std::string label,
         SDL_Color bodyColor, SDL_Color accentColor);
    void handleEvent(const SDL_Event& e);
    void draw(SDL_Renderer* renderer) const;
private:
    float value01() const;
    int cx_, cy_, radius_;
    std::function<float()> getter_;
    std::function<void(float)> setter_;
    float minV_, maxV_;
    std::string label_;
    SDL_Color bodyColor_, accentColor_;
    bool dragging_ = false;
    int dragStartY_ = 0;
    float dragStartVal01_ = 0.0f;
};
class Toggle {
public:
    Toggle(SDL_Rect rect, std::function<bool()> getter, std::function<void(bool)> setter,
           std::string label, SDL_Color onColor, SDL_Color offColor);
    void handleEvent(const SDL_Event& e);
    void draw(SDL_Renderer* renderer) const;
private:
    SDL_Rect rect_;
    std::function<bool()> getter_;
    std::function<void(bool)> setter_;
    std::string label_;
    SDL_Color onColor_, offColor_;
};
class LED {
public:
    LED(int cx, int cy, int radius, std::function<bool()> litGetter, SDL_Color onColor,
        SDL_Color offColor);
    void draw(SDL_Renderer* renderer) const;
private:
    int cx_, cy_, radius_;
    std::function<bool()> litGetter_;
    SDL_Color onColor_, offColor_;
};
class Slider {
public:
    Slider(SDL_Rect rect, std::atomic<float>* target, float minV, float maxV, std::string label,
           SDL_Color trackColor, SDL_Color handleColor);
    Slider(SDL_Rect rect, std::function<float()> getter, std::function<void(float)> setter,
           float minV, float maxV, std::string label, SDL_Color trackColor,
           SDL_Color handleColor);
    void handleEvent(const SDL_Event& e);
    void draw(SDL_Renderer* renderer) const;
private:
    float value01() const;
    SDL_Rect rect_;
    std::function<float()> getter_;
    std::function<void(float)> setter_;
    float minV_, maxV_;
    std::string label_;
    SDL_Color trackColor_, handleColor_;
    bool dragging_ = false;
};
class Button {
public:
    Button(SDL_Rect rect, std::function<void()> onClick, std::string label, SDL_Color bodyColor,
           SDL_Color textColor);
    void handleEvent(const SDL_Event& e);
    void draw(SDL_Renderer* renderer) const;
private:
    SDL_Rect rect_;
    std::function<void()> onClick_;
    std::string label_;
    SDL_Color bodyColor_, textColor_;
};
class PatchJack {
public:
    PatchJack(int cx, int cy, int radius, JackId id, std::string label, SDL_Color color);
    bool hitTest(int px, int py) const;
    JackId id() const { return id_; }
    int cx() const { return cx_; }
    int cy() const { return cy_; }
    void draw(SDL_Renderer* renderer, bool connected, bool isDragOrigin) const;
private:
    int cx_, cy_, radius_;
    JackId id_;
    std::string label_;
    SDL_Color color_;
};

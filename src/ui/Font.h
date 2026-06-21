#pragma once
#include <SDL2/SDL.h>
namespace Font {
constexpr int kGlyphW = 5;
constexpr int kGlyphH = 7;
void drawText(SDL_Renderer* renderer, int x, int y, const char* text, SDL_Color color, int scale = 2);
int textWidth(const char* text, int scale = 2);
}

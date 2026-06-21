#pragma once
#include <cmath>
inline float semitoneToHz(float semitones, float baseHz) {
    return baseHz * std::pow(2.0f, semitones / 12.0f);
}
inline float midiToHz(int midiNote) {
    return 440.0f * std::pow(2.0f, (midiNote - 69) / 12.0f);
}

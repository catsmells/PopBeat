#pragma once
#include <random>
#include "Module.h"
class NoiseSource : public Module {
public:
    float process() { return dist_(rng_); }
    void reset() override {}
private:
    std::mt19937 rng_{std::random_device{}()};
    std::uniform_real_distribution<float> dist_{-1.0f, 1.0f};
};

#pragma once
#include <atomic>
struct Param {
    const char* name;
    std::atomic<float>* value;
    float minVal;
    float maxVal;
    float defaultVal;
};

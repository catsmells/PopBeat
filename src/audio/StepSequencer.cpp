#include "StepSequencer.h"
#include <algorithm>
bool StepSequencer::advance(float& outSemitone) {
    if (!isRunning()) {
        return false;
    }
    const bool forceEdge = forceImmediateStep_.exchange(false, std::memory_order_relaxed);
    const float bpm = std::max(bpm_.load(std::memory_order_relaxed), 1.0f);
    const double samplesPerStep = (60.0 / static_cast<double>(bpm)) * sampleRate_;
    bool stepEdge = forceEdge;
    if (forceEdge) {
        sampleCounter_ = 0.0;
        currentStepIdx_.store(0, std::memory_order_relaxed);
    } else {
        sampleCounter_ += 1.0;
        if (sampleCounter_ >= samplesPerStep) {
            sampleCounter_ -= samplesPerStep;
            const int next = (currentStepIdx_.load(std::memory_order_relaxed) + 1) % kNumSteps;
            currentStepIdx_.store(next, std::memory_order_relaxed);
            stepEdge = true;
        }
    }
    const Step s = getStep(currentStepIdx_.load(std::memory_order_relaxed));
    outSemitone = s.semitone;
    return stepEdge && s.enabled;
}

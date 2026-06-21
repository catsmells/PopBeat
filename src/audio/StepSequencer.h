#pragma once
#include <array>
#include <atomic>
#include "Module.h"
struct Step {
    float semitone = 0.0f;
    bool enabled = true;
};
class StepSequencer : public Module {
public:
    static constexpr int kNumSteps = 8;
    void setSampleRate(float sr) { sampleRate_ = sr; }
    void setBpm(float bpm) { bpm_.store(bpm, std::memory_order_relaxed); }
    float getBpm() const { return bpm_.load(std::memory_order_relaxed); }
    void setRunning(bool running) {
        const bool wasRunning = running_.exchange(running, std::memory_order_relaxed);
        if (running && !wasRunning) {
            forceImmediateStep_.store(true, std::memory_order_relaxed);
        }
    }
    bool isRunning() const { return running_.load(std::memory_order_relaxed); }
    void setStep(int index, Step s) {
        steps_[static_cast<size_t>(index)].store(s, std::memory_order_relaxed);
    }
    Step getStep(int index) const {
        return steps_[static_cast<size_t>(index)].load(std::memory_order_relaxed);
    }
    bool advance(float& outSemitone);
    int currentStep() const { return currentStepIdx_.load(std::memory_order_relaxed); }
    void reset() override {
        sampleCounter_ = 0.0;
        currentStepIdx_.store(0, std::memory_order_relaxed);
    }
private:
    std::array<std::atomic<Step>, kNumSteps> steps_{};
    std::atomic<float> bpm_{120.0f};
    std::atomic<bool> running_{false};
    std::atomic<bool> forceImmediateStep_{false};
    std::atomic<int> currentStepIdx_{0};
    double sampleCounter_ = 0.0;
    float sampleRate_ = 44100.0f;
};
static_assert(std::atomic<Step>::is_always_lock_free,
              "Step must be lock-free atomic on the target platform");

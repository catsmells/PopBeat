#pragma once
#include <atomic>
#include <cstddef>
#include <string>
#include <vector>
class Recorder {
public:
    void setSampleRate(float sr) { sampleRate_ = sr; }
    void beginRecording();
    void stopRecording() { recording_.store(false, std::memory_order_release); }
    bool exportToFlac(const std::string& path) const;
    bool isRecording() const { return recording_.load(std::memory_order_acquire); }
    double elapsedSeconds() const;
    void appendIfRecording(float left, float right);
private:
    static constexpr int kMaxRecordingSeconds = 600;
    std::vector<float> buffer_;
    std::atomic<size_t> framesWritten_{0};
    std::atomic<bool> recording_{false};
    float sampleRate_ = 44100.0f;
};

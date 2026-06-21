#include "Recorder.h"
#include <FLAC/stream_encoder.h>
#include <algorithm>
#include <cstdint>
#include <filesystem>
void Recorder::beginRecording() {
    if (buffer_.empty()) {
        const size_t totalFloats = static_cast<size_t>(kMaxRecordingSeconds * sampleRate_) * 2;
        buffer_.resize(totalFloats, 0.0f);
    }
    framesWritten_.store(0, std::memory_order_relaxed);
    recording_.store(true, std::memory_order_release);
}
double Recorder::elapsedSeconds() const {
    return static_cast<double>(framesWritten_.load(std::memory_order_acquire)) / sampleRate_;
}
void Recorder::appendIfRecording(float left, float right) {
    if (!recording_.load(std::memory_order_acquire)) {
        return;
    }
    const size_t idx = framesWritten_.load(std::memory_order_relaxed);
    const size_t maxFrames = static_cast<size_t>(kMaxRecordingSeconds * sampleRate_);
    if (idx >= maxFrames) {
        recording_.store(false, std::memory_order_release);
        return;
    }
    const size_t base = idx * 2;
    buffer_[base] = left;
    buffer_[base + 1] = right;
    framesWritten_.fetch_add(1, std::memory_order_release);
}
bool Recorder::exportToFlac(const std::string& path) const {
    const size_t frameCount = framesWritten_.load(std::memory_order_acquire);
    if (frameCount == 0) {
        return(false);
    }
    const std::filesystem::path p(path);
    std::error_code ec;
    std::filesystem::create_directories(p.parent_path(), ec);
    std::vector<FLAC__int32> pcm(frameCount * 2);
    constexpr float kScale24Bit = 8388607.0f;  // 2^23 - 1
    for (size_t i = 0; i < frameCount * 2; ++i) {
        const float clamped = std::clamp(buffer_[i], -1.0f, 1.0f);
        pcm[i] = static_cast<FLAC__int32>(clamped * kScale24Bit);
    }
    FLAC__StreamEncoder* encoder = FLAC__stream_encoder_new();
    if (!encoder) {
        return(false);
    }
    FLAC__stream_encoder_set_channels(encoder, 2);
    FLAC__stream_encoder_set_bits_per_sample(encoder, 24);
    FLAC__stream_encoder_set_sample_rate(encoder, static_cast<unsigned>(sampleRate_));
    FLAC__stream_encoder_set_compression_level(encoder, 5);
    const FLAC__StreamEncoderInitStatus initStatus =
        FLAC__stream_encoder_init_file(encoder, path.c_str(), nullptr, nullptr);
    if (initStatus != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
        FLAC__stream_encoder_delete(encoder);
        return(false);
    }
    const FLAC__bool ok = FLAC__stream_encoder_process_interleaved(
        encoder, pcm.data(), static_cast<unsigned>(frameCount));
    FLAC__stream_encoder_finish(encoder);
    FLAC__stream_encoder_delete(encoder);
    return ok != 0;
}

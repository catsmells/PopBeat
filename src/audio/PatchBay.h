#pragma once
#include <array>
#include <atomic>
#include <cstdint>
#include "../voices/DfamVoice.h"
#include "../voices/Ms20Voice.h"
enum class JackId : int8_t {
    DfamVco1 = 0,
    DfamVco2,
    DfamNoise,
    DfamEg1,
    DfamEg2,
    DfamFilterOut,
    DfamTrig,
    DfamVco1Pitch,
    DfamVco2Pitch,
    DfamFilterCv,
    DfamTrigIn,
    Ms20Vco1,
    Ms20Vco2,
    Ms20Ring,
    Ms20Eg,
    Ms20Lfo,
    Ms20FilterOut,
    Ms20Gate,
    Ms20Vco1Pitch,
    Ms20Vco2Pitch,
    Ms20FilterCv,
    Ms20GateIn,
    kCount
};
constexpr int kJackCount = static_cast<int>(JackId::kCount);
constexpr int kFirstDfamInput = static_cast<int>(JackId::DfamVco1Pitch);
constexpr int kFirstMs20Output = static_cast<int>(JackId::Ms20Vco1);
constexpr int kFirstMs20Input = static_cast<int>(JackId::Ms20Vco1Pitch);
constexpr bool isSource(JackId id) {
    const int v = static_cast<int>(id);
    return (v < kFirstDfamInput) || (v >= kFirstMs20Output && v < kFirstMs20Input);
}
constexpr bool isDestination(JackId id) { return !isSource(id); }
struct PatchSlotValue {
    int8_t sourceId = -1;
    int8_t destId = -1;
};
inline int32_t packSlot(PatchSlotValue v) {
    return (static_cast<int32_t>(static_cast<uint8_t>(v.sourceId)) << 8) |
           static_cast<int32_t>(static_cast<uint8_t>(v.destId));
}
inline PatchSlotValue unpackSlot(int32_t packed) {
    return PatchSlotValue{static_cast<int8_t>((packed >> 8) & 0xFF),
                           static_cast<int8_t>(packed & 0xFF)};
}
constexpr int32_t kEmptySlot = -1;
class PatchBay {
public:
    static constexpr int kMaxCables = 24;
    PatchBay();
    bool connect(JackId source, JackId dest);
    void disconnectMostRecent(JackId jack);
    void clearAll();
    struct CableView {
        JackId source;
        JackId dest;
    };
    int getConnections(std::array<CableView, kMaxCables>& out) const;
    void tick(DfamVoice& dfam, Ms20Voice& ms20);
    void cacheOutputs(const DfamVoice& dfam, const Ms20Voice& ms20);
    void refreshSnapshotForCallback();
private:
    float readSource(JackId id) const;
    void accumulateDestination(JackId id, float v, DfamVoice& dfam, Ms20Voice& ms20);
    std::array<std::atomic<int32_t>, kMaxCables> slots_{};
    std::array<PatchSlotValue, kMaxCables> snapshot_{};
    std::array<float, kJackCount> cachedOutputs_{};
};

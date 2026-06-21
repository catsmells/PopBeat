#include "PatchBay.h"
#include <algorithm>
namespace {
constexpr float kPitchCvClampSemis = 48.0f;
constexpr float kFilterCvClampHz = 4000.0f;
constexpr float kCachedOutputClamp = 2.0f;
}
PatchBay::PatchBay() {
    for (auto& slot : slots_) {
        slot.store(kEmptySlot, std::memory_order_relaxed);
    }
}
bool PatchBay::connect(JackId source, JackId dest) {
    if (source == dest) return false;
    if (!(isSource(source) && isDestination(dest))) return false;
    for (auto& slot : slots_) {
        if (slot.load(std::memory_order_relaxed) == kEmptySlot) {
            slot.store(packSlot({static_cast<int8_t>(source), static_cast<int8_t>(dest)}),
                       std::memory_order_relaxed);
            return true;
        }
    }
    return(false);
}
void PatchBay::disconnectMostRecent(JackId jack) {
    for (int i = kMaxCables - 1; i >= 0; --i) {
        const int32_t packed = slots_[static_cast<size_t>(i)].load(std::memory_order_relaxed);
        if (packed == kEmptySlot) continue;
        const PatchSlotValue v = unpackSlot(packed);
        if (static_cast<JackId>(v.sourceId) == jack || static_cast<JackId>(v.destId) == jack) {
            slots_[static_cast<size_t>(i)].store(kEmptySlot, std::memory_order_relaxed);
            return;
        }
    }
}
void PatchBay::clearAll() {
    for (auto& slot : slots_) {
        slot.store(kEmptySlot, std::memory_order_relaxed);
    }
}
int PatchBay::getConnections(std::array<CableView, kMaxCables>& out) const {
    int n = 0;
    for (const auto& slot : slots_) {
        const int32_t packed = slot.load(std::memory_order_relaxed);
        if (packed == kEmptySlot) continue;
        const PatchSlotValue v = unpackSlot(packed);
        out[static_cast<size_t>(n++)] = {static_cast<JackId>(v.sourceId),
                                          static_cast<JackId>(v.destId)};
    }
    return(n);
}
void PatchBay::refreshSnapshotForCallback() {
    for (size_t i = 0; i < static_cast<size_t>(kMaxCables); ++i) {
        snapshot_[i] = unpackSlot(slots_[i].load(std::memory_order_relaxed));
    }
}
float PatchBay::readSource(JackId id) const { return cachedOutputs_[static_cast<size_t>(id)]; }
void PatchBay::accumulateDestination(JackId id, float v, DfamVoice& dfam, Ms20Voice& ms20) {
    switch (id) {
        case JackId::DfamVco1Pitch:
            dfam.addExtVco1Pitch(std::clamp(v, -kPitchCvClampSemis, kPitchCvClampSemis));
            break;
        case JackId::DfamVco2Pitch:
            dfam.addExtVco2Pitch(std::clamp(v, -kPitchCvClampSemis, kPitchCvClampSemis));
            break;
        case JackId::DfamFilterCv:
            dfam.addExtFilterCv(std::clamp(v, -kFilterCvClampHz, kFilterCvClampHz));
            break;
        case JackId::DfamTrigIn:
            dfam.addExtTrigIn(v);
            break;
        case JackId::Ms20Vco1Pitch:
            ms20.addExtVco1Pitch(std::clamp(v, -kPitchCvClampSemis, kPitchCvClampSemis));
            break;
        case JackId::Ms20Vco2Pitch:
            ms20.addExtVco2Pitch(std::clamp(v, -kPitchCvClampSemis, kPitchCvClampSemis));
            break;
        case JackId::Ms20FilterCv:
            ms20.addExtFilterCv(std::clamp(v, -kFilterCvClampHz, kFilterCvClampHz));
            break;
        case JackId::Ms20GateIn:
            ms20.addExtGateIn(v);
            break;
        default:
            break;
    }
}
void PatchBay::tick(DfamVoice& dfam, Ms20Voice& ms20) {
    dfam.clearExtInputs();
    ms20.clearExtInputs();
    for (const PatchSlotValue& v : snapshot_) {
        if (v.sourceId < 0 || v.destId < 0) continue;
        const float value = readSource(static_cast<JackId>(v.sourceId));
        accumulateDestination(static_cast<JackId>(v.destId), value, dfam, ms20);
    }
}
void PatchBay::cacheOutputs(const DfamVoice& dfam, const Ms20Voice& ms20) {
    auto clampCache = [](float v) { return std::clamp(v, -kCachedOutputClamp, kCachedOutputClamp); };
    cachedOutputs_[static_cast<size_t>(JackId::DfamVco1)] = clampCache(dfam.lastVco1());
    cachedOutputs_[static_cast<size_t>(JackId::DfamVco2)] = clampCache(dfam.lastVco2());
    cachedOutputs_[static_cast<size_t>(JackId::DfamNoise)] = clampCache(dfam.lastNoise());
    cachedOutputs_[static_cast<size_t>(JackId::DfamEg1)] = clampCache(dfam.lastEg1());
    cachedOutputs_[static_cast<size_t>(JackId::DfamEg2)] = clampCache(dfam.lastEg2());
    cachedOutputs_[static_cast<size_t>(JackId::DfamFilterOut)] = clampCache(dfam.lastFilterOut());
    cachedOutputs_[static_cast<size_t>(JackId::DfamTrig)] = clampCache(dfam.lastTrig());
    cachedOutputs_[static_cast<size_t>(JackId::Ms20Vco1)] = clampCache(ms20.lastVco1());
    cachedOutputs_[static_cast<size_t>(JackId::Ms20Vco2)] = clampCache(ms20.lastVco2());
    cachedOutputs_[static_cast<size_t>(JackId::Ms20Ring)] = clampCache(ms20.lastRing());
    cachedOutputs_[static_cast<size_t>(JackId::Ms20Eg)] = clampCache(ms20.lastEg());
    cachedOutputs_[static_cast<size_t>(JackId::Ms20Lfo)] = clampCache(ms20.lastLfo());
    cachedOutputs_[static_cast<size_t>(JackId::Ms20FilterOut)] = clampCache(ms20.lastFilterOut());
    cachedOutputs_[static_cast<size_t>(JackId::Ms20Gate)] = clampCache(ms20.lastGate());
}

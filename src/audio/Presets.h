#pragma once
#include <array>
#include <utility>
#include <vector>
#include "../voices/DfamVoice.h"
#include "../voices/Ms20Voice.h"
#include "PatchBay.h"
struct ParamValue {
    const char* name;
    float value;
};
struct Preset {
    const char* displayName;
    std::vector<ParamValue> dfamParams;
    std::array<Step, StepSequencer::kNumSteps> dfamSteps;
    float dfamBpm;
    std::vector<ParamValue> ms20Params;
    bool ms20LfoSyncToTempo;
    bool ms20HardSync;
    std::vector<std::pair<JackId, JackId>> patches;
    bool autoRun;
};
const std::vector<Preset>& factoryPresets();
void applyPreset(const Preset& preset, DfamVoice& dfam, Ms20Voice& ms20, PatchBay& patchBay);

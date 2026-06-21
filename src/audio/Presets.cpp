#include "Presets.h"
#include <cstring>
namespace {
void applyParams(const std::vector<ParamValue>& values, const std::vector<Param>& target) {
    for (const ParamValue& pv : values) {
        for (const Param& p : target) {
            if (std::strcmp(pv.name, p.name) == 0) {
                p.value->store(pv.value, std::memory_order_relaxed);
                break;
            }
        }
    }
}}
void applyPreset(const Preset& preset, DfamVoice& dfam, Ms20Voice& ms20, PatchBay& patchBay) {
    applyParams(preset.dfamParams, dfam.params());
    applyParams(preset.ms20Params, ms20.params());
    for (int i = 0; i < StepSequencer::kNumSteps; ++i) {
        dfam.sequencer().setStep(i, preset.dfamSteps[static_cast<size_t>(i)]);
    }
    dfam.sequencer().setBpm(preset.dfamBpm);
    ms20.lfoSyncToTempo.store(preset.ms20LfoSyncToTempo, std::memory_order_relaxed);
    ms20.hardSyncEnabled.store(preset.ms20HardSync, std::memory_order_relaxed);
    patchBay.clearAll();
    for (const auto& [source, dest] : preset.patches) {
        patchBay.connect(source, dest);
    }
    dfam.sequencer().setRunning(preset.autoRun);
}
const std::vector<Preset>& factoryPresets() {
    static const std::vector<Preset> presets = {
        Preset{
            "RESET",
            {
                {"VCO1 Tune", 0.0f}, {"VCO2 Tune", 0.0f}, {"Mix VCO1", 0.7f}, {"Mix VCO2", 0.0f},
                {"Mix Noise", 0.0f}, {"Filter Cutoff", 800.0f}, {"Filter Resonance", 0.3f},
                {"EG1 Attack", 2.0f}, {"EG1 Decay", 150.0f}, {"EG2 Attack", 2.0f},
                {"EG2 Decay", 300.0f}, {"EG2->Pitch", 0.0f}, {"EG2->Filter", 0.0f},
            },
            {Step{0, true}, Step{0, true}, Step{7, true}, Step{0, true}, Step{12, true},
             Step{0, true}, Step{7, true}, Step{3, true}},
            120.0f,
            {
                {"VCO1 Tune", 0.0f}, {"VCO2 Tune", -12.0f}, {"Mix VCO1", 0.7f},
                {"Mix VCO2", 0.3f}, {"Ring Mod", 0.0f}, {"HPF Cutoff", 80.0f},
                {"LPF Cutoff", 2000.0f}, {"LPF Resonance", 0.3f}, {"EG Attack", 5.0f},
                {"EG Decay", 100.0f}, {"EG Sustain", 0.7f}, {"EG Release", 200.0f},
                {"LFO Rate", 4.0f}, {"LFO->Pitch", 0.0f}, {"LFO->Filter", 0.0f},
            },
            false, false,
            {},
            false,
        },
        Preset{
            "DA FUNK",
            {
                {"Mix VCO1", 0.0f}, {"Mix VCO2", 0.0f}, {"Mix Noise", 0.5f},
                {"Filter Cutoff", 1200.0f}, {"Filter Resonance", 0.4f}, {"EG1 Attack", 0.5f},
                {"EG1 Decay", 60.0f}, {"EG2 Attack", 0.5f}, {"EG2 Decay", 40.0f},
                {"EG2->Filter", 1500.0f},
            },
            {Step{0, true}, Step{0, false}, Step{0, true}, Step{0, true}, Step{0, false},
             Step{0, true}, Step{0, true}, Step{0, false}},
            110.0f,
            {
                {"VCO1 Tune", 0.0f}, {"VCO2 Tune", -12.0f}, {"Mix VCO1", 0.6f},
                {"Mix VCO2", 0.4f}, {"Ring Mod", 0.1f}, {"HPF Cutoff", 60.0f},
                {"LPF Cutoff", 550.0f}, {"LPF Resonance", 0.65f}, {"EG Attack", 2.0f},
                {"EG Decay", 120.0f}, {"EG Sustain", 0.25f}, {"EG Release", 90.0f},
                {"LFO Rate", 4.0f}, {"LFO->Pitch", 0.0f}, {"LFO->Filter", 900.0f},
            },
            true, false,
            {{JackId::DfamTrig, JackId::Ms20GateIn}},
            true,
        },
        Preset{
            "ACID LINE",
            {
                {"Mix VCO1", 0.0f}, {"Mix VCO2", 0.0f}, {"Mix Noise", 0.6f},
                {"Filter Cutoff", 2500.0f}, {"Filter Resonance", 0.2f}, {"EG1 Attack", 0.5f},
                {"EG1 Decay", 30.0f}, {"EG2 Attack", 0.5f}, {"EG2 Decay", 20.0f},
                {"EG2->Filter", 2000.0f},
            },
            {Step{0, true}, Step{0, true}, Step{0, true}, Step{0, true}, Step{0, true},
             Step{0, true}, Step{0, true}, Step{0, true}},
            132.0f,
            {
                {"VCO1 Tune", 0.0f}, {"VCO2 Tune", 0.0f}, {"Mix VCO1", 1.0f}, {"Mix VCO2", 0.0f},
                {"Ring Mod", 0.0f}, {"HPF Cutoff", 40.0f}, {"LPF Cutoff", 350.0f},
                {"LPF Resonance", 0.85f}, {"EG Attack", 0.5f}, {"EG Decay", 80.0f},
                {"EG Sustain", 0.0f}, {"EG Release", 30.0f}, {"LFO Rate", 0.3f},
                {"LFO->Pitch", 0.0f}, {"LFO->Filter", 2500.0f},
            },
            false, false,
            {{JackId::DfamTrig, JackId::Ms20GateIn}},
            true,
        },
        Preset{
            "AMBIENT PAD",
            {
                {"Mix VCO1", 0.0f}, {"Mix VCO2", 0.0f}, {"Mix Noise", 0.0f},
            },
            {Step{0, true}, Step{0, true}, Step{0, true}, Step{0, true}, Step{0, true},
             Step{0, true}, Step{0, true}, Step{0, true}},
            120.0f,
            {
                {"VCO1 Tune", 0.0f}, {"VCO2 Tune", 0.0f}, {"Mix VCO1", 0.5f}, {"Mix VCO2", 0.5f},
                {"Ring Mod", 0.0f}, {"HPF Cutoff", 20.0f}, {"LPF Cutoff", 3000.0f},
                {"LPF Resonance", 0.1f}, {"EG Attack", 800.0f}, {"EG Decay", 1000.0f},
                {"EG Sustain", 0.8f}, {"EG Release", 2000.0f}, {"LFO Rate", 0.5f},
                {"LFO->Pitch", 0.0f}, {"LFO->Filter", 300.0f},
            },
            false, false,
            {},
            false,
        },
    };
    return(presets);
}

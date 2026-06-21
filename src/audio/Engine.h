#pragma once
#include <atomic>
#include "../voices/DfamVoice.h"
#include "../voices/Ms20Voice.h"
#include "PatchBay.h"
#include "Recorder.h"
#include "Smoother.h"
class Engine {
public:
    void init(float sampleRate);
    void audioCallback(float* outStream, int numFrames);
    DfamVoice& dfam() { return dfamVoice_; }
    Ms20Voice& ms20() { return ms20Voice_; }
    Recorder& recorder() { return recorder_; }
    PatchBay& patchBay() { return patchBay_; }
    std::atomic<float> masterVolume{0.8f};
private:
    DfamVoice dfamVoice_;
    Ms20Voice ms20Voice_;
    Recorder recorder_;
    PatchBay patchBay_;
    OnePoleSmoother masterVolSmoother_;
};

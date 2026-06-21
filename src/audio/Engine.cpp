#include "Engine.h"
#if defined(__x86_64__) || defined(__i386__)
#include <pmmintrin.h>
#include <xmmintrin.h>
#define POPBEAT_HAS_SSE 1
#endif
void Engine::init(float sampleRate) {
    dfamVoice_.setSampleRate(sampleRate);
    ms20Voice_.setSampleRate(sampleRate);
    recorder_.setSampleRate(sampleRate);
    masterVolSmoother_.setTimeMs(10.0f, sampleRate);
    masterVolSmoother_.snap(masterVolume.load(std::memory_order_relaxed));
}
void Engine::audioCallback(float* outStream, int numFrames) {
#if defined(POPBEAT_HAS_SSE)
    static thread_local bool denormalsConfigured = false;
    if (!denormalsConfigured) {
        _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
        _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
        denormalsConfigured = true;
    }
#endif
    const float vol = masterVolume.load(std::memory_order_relaxed);
    ms20Voice_.setExternalTempo(dfamVoice_.sequencer().getBpm());
    patchBay_.refreshSnapshotForCallback();
    for (int i = 0; i < numFrames; ++i) {
        patchBay_.tick(dfamVoice_, ms20Voice_);
        const float mix = dfamVoice_.process() + ms20Voice_.process();
        patchBay_.cacheOutputs(dfamVoice_, ms20Voice_);
        const float out = mix * masterVolSmoother_.process(vol);
        outStream[i * 2 + 0] = out;
        outStream[i * 2 + 1] = out;
        recorder_.appendIfRecording(out, out);
    }
}

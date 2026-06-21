#include <SDL2/SDL.h>
#include "audio/Engine.h"
#include "ui/App.h"
namespace {
constexpr int kWindowW = 1180;
constexpr int kWindowH = 740;
constexpr float kSampleRate = 44100.0f;
constexpr int kBufferSamples = 256;
void sdlAudioCallback(void* userdata, Uint8* stream, int len) {
    Engine* engine = static_cast<Engine*>(userdata);
    const int numFrames = len / (2 * static_cast<int>(sizeof(float)));
    engine->audioCallback(reinterpret_cast<float*>(stream), numFrames);
}}
int main() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return(1);
    }
    Engine engine;
    engine.init(kSampleRate);
    SDL_AudioSpec want{};
    SDL_AudioSpec have{};
    want.freq = static_cast<int>(kSampleRate);
    want.format = AUDIO_F32SYS;
    want.channels = 2;
    want.samples = kBufferSamples;
    want.callback = sdlAudioCallback;
    want.userdata = &engine;
    const SDL_AudioDeviceID audioDevice =
        SDL_OpenAudioDevice(nullptr, 0, &want, &have,
                            SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
    if (audioDevice == 0) {
        SDL_Log("SDL_OpenAudioDevice failed: %s", SDL_GetError());
        return(1);
    }
    SDL_PauseAudioDevice(audioDevice, 0);
    SDL_Window* window =
        SDL_CreateWindow("PopBeat", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, kWindowW,
                          kWindowH, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        return(1);
    }
    SDL_SetWindowMinimumSize(window, kWindowW, kWindowH);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    }
    if (!renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        return(1);
    }
    App app(renderer, engine, kWindowW, kWindowH);
    bool quit = false;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else {
                app.handleEvent(e);
            }
        }
        app.render(renderer);
        SDL_RenderPresent(renderer);
        SDL_Delay(16);}
    SDL_CloseAudioDevice(audioDevice);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return(0);
}

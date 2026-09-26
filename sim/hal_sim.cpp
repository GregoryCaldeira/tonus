#include "hal_sim.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>

#include "version.hpp"

namespace tonus {

SimHal::SimHal(std::string settingsPath) : settingsPath_(std::move(settingsPath)) { loadSettingsFile(); }

SimHal::~SimHal() {
    if (in_) SDL_CloseAudioDevice(in_);
    if (out_) SDL_CloseAudioDevice(out_);
}

bool SimHal::initAudio() {
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0) {
        std::fprintf(stderr, "[sim] SDL audio init failed: %s\n", SDL_GetError());
        return false;
    }
    SDL_AudioSpec want{};
    want.freq = kSampleRate;
    want.format = AUDIO_S16SYS;
    want.channels = 1;
    want.samples = 256;
    want.callback = playbackCallback;
    want.userdata = this;
    out_ = SDL_OpenAudioDevice(nullptr, 0, &want, nullptr, 0);
    if (!out_) {
        std::fprintf(stderr, "[sim] no audio output: %s\n", SDL_GetError());
        return false;
    }
    SDL_PauseAudioDevice(out_, 0);
    return true;
}

DeviceInfo SimHal::info() const {
    SDL_version v;
    SDL_GetVersion(&v);
    char sdl[32];
    std::snprintf(sdl, sizeof(sdl), "SDL %d.%d.%d", v.major, v.minor, v.patch);
    return {"sim", sdl, kAppVersion, "SDL window"};
}

void SimHal::playPcm(std::vector<int16_t> mono, int /*sampleRate*/) {
    std::lock_guard<std::mutex> lock(audioMutex_);
    pcm_ = std::move(mono);
    pcmPos_ = 0;
    tone_ = false;
}

void SimHal::startTone(float hz) {
    std::lock_guard<std::mutex> lock(audioMutex_);
    osc_.set(hz, kSampleRate);
    tone_ = true;
    pcm_.clear();
}

void SimHal::stopTone() {
    std::lock_guard<std::mutex> lock(audioMutex_);
    tone_ = false;
}

void SimHal::playbackCallback(void* userdata, Uint8* stream, int len) {
    auto* self = static_cast<SimHal*>(userdata);
    auto* out = reinterpret_cast<int16_t*>(stream);
    const size_t frames = static_cast<size_t>(len) / sizeof(int16_t);
    std::lock_guard<std::mutex> lock(self->audioMutex_);
    if (self->tone_) {
        self->osc_.render(out, frames, 1, 0.3f);
        return;
    }
    const size_t n = std::min(frames, self->pcm_.size() - std::min(self->pcmPos_, self->pcm_.size()));
    std::copy_n(self->pcm_.begin() + static_cast<long>(self->pcmPos_), n, out);
    std::fill(out + n, out + frames, 0);
    self->pcmPos_ += n;
}

void SimHal::setMicEnabled(bool enabled) {
    if (enabled && !in_) {
        SDL_AudioSpec want{};
        want.freq = kSampleRate;
        want.format = AUDIO_S16SYS;
        want.channels = 1;
        want.samples = 256;
        want.callback = captureCallback;
        want.userdata = this;
        in_ = SDL_OpenAudioDevice(nullptr, 1, &want, nullptr, 0);
        if (!in_) std::fprintf(stderr, "[sim] no microphone: %s\n", SDL_GetError());
    }
    if (in_) SDL_PauseAudioDevice(in_, enabled ? 0 : 1);
    if (!enabled) micLevel_ = 0.0f;
}

void SimHal::captureCallback(void* userdata, Uint8* stream, int len) {
    auto* self = static_cast<SimHal*>(userdata);
    const auto* in = reinterpret_cast<const int16_t*>(stream);
    const size_t n = static_cast<size_t>(len) / sizeof(int16_t);
    double sum = 0;
    for (size_t i = 0; i < n; ++i) sum += double(in[i]) * in[i];
    const float rms = n ? static_cast<float>(std::sqrt(sum / n) / 32768.0) : 0.0f;
    const float level = self->micLevel_;
    self->micLevel_ = rms > level ? rms : level * 0.9f + rms * 0.1f;
}

int SimHal::loadSetting(const char* key, int fallback) {
    const auto it = settings_.find(key);
    return it == settings_.end() ? fallback : it->second;
}

void SimHal::saveSetting(const char* key, int value) {
    settings_[key] = value;
    saveSettingsFile();
}

void SimHal::loadSettingsFile() {
    if (settingsPath_.empty()) return;
    std::ifstream f(settingsPath_);
    std::string key;
    int value;
    while (f >> key >> value) settings_[key] = value;
}

void SimHal::saveSettingsFile() const {
    if (settingsPath_.empty()) return;
    std::ofstream f(settingsPath_);
    for (const auto& [k, v] : settings_) f << k << ' ' << v << '\n';
}

}  // namespace tonus

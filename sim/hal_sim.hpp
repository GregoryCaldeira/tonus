#pragma once
// DeviceHal for the desktop simulator: SDL audio out/capture, settings in a small text file.

#include <atomic>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include <SDL2/SDL.h>

#include "audio/chiptune.hpp"
#include "device_hal.hpp"

namespace tonus {

class SimHal final : public DeviceHal {
public:
    static constexpr int kSampleRate = 48000;

    explicit SimHal(std::string settingsPath);
    ~SimHal() override;

    bool initAudio();

    DeviceInfo info() const override;
    size_t freeInternalHeap() const override { return 0; }
    size_t freePsram() const override { return 0; }

    void playPcm(std::vector<int16_t> mono, int sampleRate) override;
    void startTone(float hz) override;
    void stopTone() override;

    void setMicEnabled(bool enabled) override;
    float micLevel() override { return micLevel_; }

    int loadSetting(const char* key, int fallback) override;
    void saveSetting(const char* key, int value) override;

private:
    static void playbackCallback(void* self, Uint8* stream, int len);
    static void captureCallback(void* self, Uint8* stream, int len);
    void loadSettingsFile();
    void saveSettingsFile() const;

    SDL_AudioDeviceID out_ = 0;
    SDL_AudioDeviceID in_ = 0;

    std::mutex audioMutex_;  // sim only: guards the fields below against the SDL audio thread
    std::vector<int16_t> pcm_;
    size_t pcmPos_ = 0;
    bool tone_ = false;
    audio::SineOsc osc_;

    std::atomic<float> micLevel_{0.0f};
    std::string settingsPath_;
    std::map<std::string, int> settings_;
};

}  // namespace tonus

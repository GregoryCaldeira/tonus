#pragma once
// DeviceHal for the M5Stack Tab5: ES8388 speaker/jack out, ES7210 mic in, NVS settings.

#include <atomic>

#include "device_hal.hpp"
#include "esp_codec_dev.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "nvs.h"

namespace tonus {

class Tab5Hal final : public DeviceHal {
public:
    static constexpr int kSampleRate = 48000;

    bool initSettings();
    bool initSpeaker();
    bool initMic();
    bool mountStorage();
    void detectPanel();  // call after the display is up (the I2C bus exists)

    DeviceInfo info() const override;
    size_t freeInternalHeap() const override;
    size_t freePsram() const override;

    void playPcm(std::vector<int16_t> mono, int sampleRate) override;
    void startTone(float hz) override;
    void stopTone() override;

    void setMicEnabled(bool enabled) override;
    float micLevel() override;

    int loadSetting(const char* key, int fallback) override;
    void saveSetting(const char* key, int value) override;

private:
    struct AudioCmd {
        enum Kind : uint8_t { Stop, Pcm, Tone } kind;
        float hz;
        std::vector<int16_t>* pcm;  // owned by the receiver
    };

    static void audioTask(void* arg);
    static void micTask(void* arg);

    esp_codec_dev_handle_t speaker_ = nullptr;
    esp_codec_dev_handle_t mic_ = nullptr;
    QueueHandle_t audioQueue_ = nullptr;
    TaskHandle_t micTaskHandle_ = nullptr;
    nvs_handle_t nvs_ = 0;
    std::atomic<bool> micEnabled_{false};
    std::atomic<float> micLevel_{0.0f};
    const char* panel_ = "";
};

}  // namespace tonus

#include "hal_tab5.hpp"

#include <cmath>
#include <cstdio>

#include "audio/chiptune.hpp"
#include "bsp/m5stack_tab5.h"
#include "driver/i2c_master.h"
#include "esp_chip_info.h"
#include "esp_heap_caps.h"
#include "esp_littlefs.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "version.hpp"

namespace tonus {
namespace {

constexpr const char* TAG = "tonus.hal";
constexpr int kBlockFrames = 256;
constexpr int kSpeakerVolume = 60;   // % — the jingle itself is normalised to -4.4 dBFS
constexpr float kMicGainDb = 30.0f;
constexpr float kToneGain = 0.3f;

// I2C addresses the BSP uses to tell the panel generations apart (see bsp_display.c).
constexpr uint16_t kSt7123TouchAddr = 0x55;
constexpr uint16_t kGt911TouchAddrBackup = 0x14;

}  // namespace

bool Tab5Hal::initSettings() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition changed, erasing");
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err == ESP_OK) err = nvs_open("tonus", NVS_READWRITE, &nvs_);
    if (err != ESP_OK) ESP_LOGE(TAG, "NVS init failed: %s", esp_err_to_name(err));
    return err == ESP_OK;
}

bool Tab5Hal::initSpeaker() {
    speaker_ = bsp_audio_codec_speaker_init();
    if (!speaker_) {
        ESP_LOGE(TAG, "Speaker codec init failed");
        return false;
    }
    esp_codec_dev_sample_info_t fs = {};
    fs.sample_rate = kSampleRate;
    fs.channel = 1;
    fs.bits_per_sample = 16;
    if (esp_codec_dev_open(speaker_, &fs) != ESP_CODEC_DEV_OK) {
        ESP_LOGE(TAG, "Speaker open failed");
        return false;
    }
    esp_codec_dev_set_out_vol(speaker_, kSpeakerVolume);

    audioQueue_ = xQueueCreate(4, sizeof(AudioCmd));
    xTaskCreatePinnedToCore(audioTask, "tonus_audio", 6144, this, 10, nullptr, 1);
    return true;
}

bool Tab5Hal::initMic() {
    mic_ = bsp_audio_codec_microphone_init();
    if (!mic_) {
        ESP_LOGE(TAG, "Microphone codec init failed");
        return false;
    }
    esp_codec_dev_sample_info_t fs = {};
    fs.sample_rate = kSampleRate;
    fs.channel = 1;
    fs.bits_per_sample = 16;
    if (esp_codec_dev_open(mic_, &fs) != ESP_CODEC_DEV_OK) {
        ESP_LOGE(TAG, "Microphone open failed");
        return false;
    }
    esp_codec_dev_set_in_gain(mic_, kMicGainDb);
    xTaskCreatePinnedToCore(micTask, "tonus_mic", 4096, this, 8, &micTaskHandle_, 1);
    return true;
}

bool Tab5Hal::mountStorage() {
    esp_vfs_littlefs_conf_t conf = {};
    conf.base_path = "/data";
    conf.partition_label = "storage";
    conf.format_if_mount_failed = true;
    const esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "LittleFS mount failed: %s", esp_err_to_name(err));
        return false;
    }
    size_t total = 0, used = 0;
    esp_littlefs_info(conf.partition_label, &total, &used);
    ESP_LOGI(TAG, "LittleFS mounted at /data: %u KB used of %u KB", unsigned(used / 1024), unsigned(total / 1024));
    return true;
}

void Tab5Hal::detectPanel() {
    i2c_master_bus_handle_t bus = bsp_i2c_get_handle();
    if (!bus) return;
    if (i2c_master_probe(bus, kSt7123TouchAddr, 50) == ESP_OK) panel_ = "ST712x";
    else if (i2c_master_probe(bus, kGt911TouchAddrBackup, 50) == ESP_OK) panel_ = "ILI9881C";
    ESP_LOGI(TAG, "Display panel: %s", *panel_ ? panel_ : "unknown");
}

DeviceInfo Tab5Hal::info() const {
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    char rev[16];
    std::snprintf(rev, sizeof(rev), "v%d.%d", chip.revision / 100, chip.revision % 100);
    return {rev, esp_get_idf_version(), kAppVersion, panel_};
}

size_t Tab5Hal::freeInternalHeap() const { return heap_caps_get_free_size(MALLOC_CAP_INTERNAL); }
size_t Tab5Hal::freePsram() const { return heap_caps_get_free_size(MALLOC_CAP_SPIRAM); }

void Tab5Hal::playPcm(std::vector<int16_t> mono, int sampleRate) {
    if (!audioQueue_) return;
    if (sampleRate != kSampleRate) ESP_LOGW(TAG, "playPcm: %d Hz requested, playing at %d Hz", sampleRate, kSampleRate);
    AudioCmd cmd{AudioCmd::Pcm, 0.0f, new std::vector<int16_t>(std::move(mono))};
    if (xQueueSend(audioQueue_, &cmd, 0) != pdTRUE) delete cmd.pcm;
}

void Tab5Hal::startTone(float hz) {
    if (!audioQueue_) return;
    AudioCmd cmd{AudioCmd::Tone, hz, nullptr};
    xQueueSend(audioQueue_, &cmd, 0);
}

void Tab5Hal::stopTone() {
    if (!audioQueue_) return;
    AudioCmd cmd{AudioCmd::Stop, 0.0f, nullptr};
    xQueueSend(audioQueue_, &cmd, 0);
}

// Playback task: owns the current PCM buffer or tone; blocks on the queue while idle.
void Tab5Hal::audioTask(void* arg) {
    auto* self = static_cast<Tab5Hal*>(arg);
    int16_t block[kBlockFrames];
    std::vector<int16_t>* pcm = nullptr;
    size_t pos = 0;
    bool tone = false;
    audio::SineOsc osc;

    for (;;) {
        AudioCmd cmd;
        const bool idle = !pcm && !tone;
        if (xQueueReceive(self->audioQueue_, &cmd, idle ? portMAX_DELAY : 0) == pdTRUE) {
            delete pcm;
            pcm = nullptr;
            tone = false;
            if (cmd.kind == AudioCmd::Pcm) {
                pcm = cmd.pcm;
                pos = 0;
            } else if (cmd.kind == AudioCmd::Tone) {
                osc.set(cmd.hz, kSampleRate);
                tone = true;
            }
            continue;
        }

        if (tone) {
            osc.render(block, kBlockFrames, 1, kToneGain);
        } else {
            const size_t n = std::min<size_t>(kBlockFrames, pcm->size() - pos);
            std::copy(pcm->begin() + pos, pcm->begin() + pos + n, block);
            std::fill(block + n, block + kBlockFrames, 0);
            pos += n;
            if (pos >= pcm->size()) {
                delete pcm;
                pcm = nullptr;
            }
        }
        esp_codec_dev_write(self->speaker_, block, sizeof(block));
    }
}

void Tab5Hal::setMicEnabled(bool enabled) {
    micEnabled_ = enabled;
    if (!enabled) micLevel_ = 0.0f;
    if (enabled && micTaskHandle_) xTaskNotifyGive(micTaskHandle_);
}

float Tab5Hal::micLevel() { return micLevel_; }

// Capture task: computes a smoothed RMS level while the meter is visible.
void Tab5Hal::micTask(void* arg) {
    auto* self = static_cast<Tab5Hal*>(arg);
    int16_t block[kBlockFrames];
    float level = 0;
    for (;;) {
        if (!self->micEnabled_) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
            level = 0;
            continue;
        }
        if (esp_codec_dev_read(self->mic_, block, sizeof(block)) != ESP_CODEC_DEV_OK) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }
        double sum = 0;
        for (int16_t s : block) sum += double(s) * s;
        const float rms = static_cast<float>(std::sqrt(sum / kBlockFrames) / 32768.0);
        // Fast attack, slower release, so the meter reads like a VU.
        level = rms > level ? rms : level * 0.9f + rms * 0.1f;
        self->micLevel_ = level;
    }
}

int Tab5Hal::loadSetting(const char* key, int fallback) {
    int32_t v = fallback;
    if (nvs_ && nvs_get_i32(nvs_, key, &v) != ESP_OK) v = fallback;
    return static_cast<int>(v);
}

void Tab5Hal::saveSetting(const char* key, int value) {
    if (!nvs_) return;
    nvs_set_i32(nvs_, key, value);
    nvs_commit(nvs_);
}

}  // namespace tonus

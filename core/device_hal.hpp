#pragma once
// Hardware abstraction used by the UI and app layers. Implemented by firmware/main/hal_tab5.cpp
// (M5Stack Tab5) and sim/hal_sim.cpp (desktop SDL). Nothing here may include platform headers.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace tonus {

struct DeviceInfo {
    std::string chipRevision;  // e.g. "v1.3"
    std::string idfVersion;    // e.g. "v6.1" (or "sim")
    std::string appVersion;    // e.g. "0.1.0"
    std::string panel;         // display controller, e.g. "ST7123" or "ILI9881C"
};

class DeviceHal {
public:
    virtual ~DeviceHal() = default;

    virtual DeviceInfo info() const = 0;
    virtual size_t freeInternalHeap() const = 0;
    virtual size_t freePsram() const = 0;

    // Plays mono PCM once, asynchronously (the HAL keeps its own copy). Stops any test tone.
    virtual void playPcm(std::vector<int16_t> mono, int sampleRate) = 0;
    // Continuous sine test tone until stopTone() or playPcm().
    virtual void startTone(float hz) = 0;
    virtual void stopTone() = 0;

    // Microphone capture for the level meter. micLevel() returns smoothed RMS in 0..1.
    virtual void setMicEnabled(bool enabled) = 0;
    virtual float micLevel() = 0;

    // Small persistent integer settings (NVS on the device).
    virtual int loadSetting(const char* key, int fallback) = 0;
    virtual void saveSetting(const char* key, int value) = 0;
};

}  // namespace tonus

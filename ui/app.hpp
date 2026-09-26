#pragma once
// UI entry point shared by the firmware and the simulator. All functions must be called from
// the LVGL thread (on the device: with bsp_display_lock held).

#include "app/boot.hpp"
#include "device_hal.hpp"

namespace tonus::ui {

enum class Screen { Splash, Home, Diagnostics };

// Settings keys stored through DeviceHal.
inline constexpr const char* kSettingLang = "lang";
inline constexpr const char* kSettingTheme = "theme";
inline constexpr const char* kSettingReduceMotion = "reduce_motion";

// Applies saved language/theme and shows the splash, which follows `boot` progress.
void start(DeviceHal& hal, const app::BootStatus& boot);

void show(Screen screen);

DeviceHal& hal();
const app::BootStatus& bootStatus();

}  // namespace tonus::ui

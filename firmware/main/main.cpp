// Tonus firmware entry point (M5Stack Tab5).
// Brings the display up in landscape, shows the splash, then runs the boot steps in a task while
// the splash animates their progress.

#include "app.hpp"
#include "app/boot.hpp"
#include "audio/chiptune.hpp"
#include "bsp/m5stack_tab5.h"
#include "driver/i2c_master.h"
#include "esp_chip_info.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal_tab5.hpp"
#include "version.hpp"

namespace {

constexpr const char* TAG = "tonus";

tonus::Tab5Hal g_hal;
tonus::app::BootStatus g_boot;

void bootTask(void*) {
    using tonus::i18n::Str;
    const std::vector<tonus::app::BootStep> steps = {
        {Str::boot_stage, [] { return g_hal.mountStorage(); }},
        {Str::boot_amps, [] { return g_hal.initSpeaker(); }},
        {Str::boot_count,
         [] {
             g_hal.playPcm(tonus::audio::renderBootJingle(tonus::Tab5Hal::kSampleRate), tonus::Tab5Hal::kSampleRate);
             return true;
         }},
        {Str::boot_mic, [] { return g_hal.initMic(); }},
    };
    tonus::app::runBoot(steps, g_boot);
    ESP_LOGI(TAG, "Boot finished%s", g_boot.failedLabel >= 0 ? " with errors" : "");
    vTaskDelete(nullptr);
}

void logSystem() {
    esp_chip_info_t chip;
    esp_chip_info(&chip);
    ESP_LOGI(TAG, "Tonus %s | ESP-IDF %s | ESP32-P4 rev v%d.%d, %d cores", tonus::kAppVersion,
             esp_get_idf_version(), chip.revision / 100, chip.revision % 100, chip.cores);
}

}  // namespace

// The LCD/touch enable lines live on an I/O expander that keeps its state across a chip reset
// (USB reset, watchdog, crash). On ST712x panels touch and display share one chip, so if a previous
// run left the LCD held in reset, the BSP's board-version probe finds no touch controller and
// asserts. Power-cycle both lines before the BSP starts the display.
void powerCyclePanel() {
    bsp_i2c_init();
    bsp_feature_enable(BSP_FEATURE_TOUCH, false);
    bsp_feature_enable(BSP_FEATURE_LCD, false);
    vTaskDelay(pdMS_TO_TICKS(50));
    bsp_feature_enable(BSP_FEATURE_LCD, true);
    vTaskDelay(pdMS_TO_TICKS(50));
    bsp_feature_enable(BSP_FEATURE_TOUCH, true);
    vTaskDelay(pdMS_TO_TICKS(300));

    if (esp_log_level_get(TAG) >= ESP_LOG_DEBUG) {
        i2c_master_bus_handle_t bus = bsp_i2c_get_handle();
        for (uint16_t addr = 0x08; addr < 0x78; ++addr)
            if (i2c_master_probe(bus, addr, 20) == ESP_OK) ESP_LOGD(TAG, "I2C device at 0x%02x", addr);
    }
}

extern "C" void app_main(void) {
    logSystem();
    powerCyclePanel();
    // Settings first: the splash needs the saved language and theme.
    const bool settingsOk = g_hal.initSettings();

    bsp_display_cfg_t cfg = {};
    cfg.lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    cfg.lvgl_port_cfg.task_affinity = 0;  // UI on core 0; audio runs on core 1
    cfg.lvgl_port_cfg.task_stack = 12 * 1024;
    // Partial draw buffers in PSRAM (keeps internal DMA RAM free for SDIO); PPA rotates them.
    cfg.buffer_size = BSP_LCD_H_RES * 160;
    cfg.double_buffer = true;
    cfg.flags.buff_dma = true;
    cfg.flags.buff_spiram = true;
    cfg.flags.sw_rotate = true;

    lv_display_t* disp = bsp_display_start_with_config(&cfg);
    if (!disp) {
        ESP_LOGE(TAG, "Display init failed");
        return;
    }
    g_hal.detectPanel();

    bsp_display_lock(0);
    bsp_display_rotate(disp, LV_DISPLAY_ROTATION_90);
    tonus::ui::start(g_hal, g_boot);
    lv_refr_now(disp);  // render the first splash frame before the backlight comes on
    bsp_display_unlock();
    bsp_display_brightness_set(80);

    if (!settingsOk) ESP_LOGW(TAG, "Running without persistent settings");
    xTaskCreatePinnedToCore(bootTask, "tonus_boot", 8192, nullptr, 5, nullptr, 1);
}

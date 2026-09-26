// Tonus desktop simulator.
//
//   tonus_sim [--screen splash|home|diag] [--lang en|pt] [--theme stage|bedroom]
//             [--screenshot out.bmp --at MS] [--settings FILE]
//
// With --screenshot the simulator renders for MS milliseconds, saves the screen as a 24-bit BMP
// and exits (used to check layouts without a device). --lang/--theme override saved settings.

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <thread>

#include <SDL2/SDL.h>

#include "app.hpp"
#include "app/boot.hpp"
#include "audio/chiptune.hpp"
#include "hal_sim.hpp"
#include "i18n/i18n.hpp"
#include "lvgl.h"
#include "theme.hpp"

namespace {

struct Options {
    std::string screen = "splash";
    std::string lang;
    std::string theme;
    std::string screenshot;
    uint32_t atMs = 3500;
    std::string settings;
};

Options parse(int argc, char** argv) {
    Options o;
    const char* home = std::getenv("HOME");
    o.settings = home ? std::string(home) + "/.tonus_sim_settings" : "";
    for (int i = 1; i + 1 < argc; i += 2) {
        const std::string k = argv[i], v = argv[i + 1];
        if (k == "--screen") o.screen = v;
        else if (k == "--lang") o.lang = v;
        else if (k == "--theme") o.theme = v;
        else if (k == "--screenshot") o.screenshot = v;
        else if (k == "--at") o.atMs = static_cast<uint32_t>(std::stoul(v));
        else if (k == "--settings") o.settings = v;
        else std::fprintf(stderr, "[sim] unknown option %s\n", k.c_str());
    }
    return o;
}

// Writes the active screen as a bottom-up 24-bit BMP (LVGL RGB888 is already B,G,R in memory).
bool saveScreenshot(const std::string& path) {
    lv_draw_buf_t* buf = lv_snapshot_take(lv_screen_active(), LV_COLOR_FORMAT_RGB888);
    if (!buf) return false;
    const uint32_t w = buf->header.w, h = buf->header.h, stride = buf->header.stride;
    const uint32_t rowBytes = (w * 3 + 3) & ~3u;
    const uint32_t dataSize = rowBytes * h;
    std::ofstream f(path, std::ios::binary);
    auto u32 = [&](uint32_t v) { f.write(reinterpret_cast<const char*>(&v), 4); };
    auto u16 = [&](uint16_t v) { f.write(reinterpret_cast<const char*>(&v), 2); };
    f.write("BM", 2);
    u32(54 + dataSize); u32(0); u32(54);
    u32(40); u32(w); u32(h); u16(1); u16(24); u32(0); u32(dataSize); u32(2835); u32(2835); u32(0); u32(0);
    const char pad[3] = {0, 0, 0};
    for (uint32_t y = h; y-- > 0;) {
        f.write(reinterpret_cast<const char*>(buf->data + y * stride), w * 3);
        f.write(pad, rowBytes - w * 3);
    }
    lv_draw_buf_destroy(buf);
    return static_cast<bool>(f);
}

}  // namespace

int main(int argc, char** argv) {
    const Options opt = parse(argc, argv);
    tonus::SimHal hal(opt.settings);
    if (!opt.lang.empty()) hal.saveSetting(tonus::ui::kSettingLang, opt.lang == "pt" ? 1 : 0);
    if (!opt.theme.empty()) hal.saveSetting(tonus::ui::kSettingTheme, opt.theme == "bedroom" ? 1 : 0);

    lv_init();
    lv_display_t* disp = lv_sdl_window_create(tonus::ui::kScreenW, tonus::ui::kScreenH);
    lv_sdl_window_set_title(disp, "Tonus simulator");
    lv_sdl_mouse_create();

    // Boot steps with realistic pacing, on a worker thread like the device's boot task.
    static tonus::app::BootStatus boot;
    tonus::ui::start(hal, boot);
    std::thread([&hal] {
        using tonus::i18n::Str;
        using namespace std::chrono_literals;
        const std::vector<tonus::app::BootStep> steps = {
            {Str::boot_stage, [] { std::this_thread::sleep_for(350ms); return true; }},
            {Str::boot_amps, [&hal] { std::this_thread::sleep_for(250ms); return hal.initAudio(); }},
            {Str::boot_count,
             [&hal] {
                 hal.playPcm(tonus::audio::renderBootJingle(tonus::SimHal::kSampleRate), tonus::SimHal::kSampleRate);
                 std::this_thread::sleep_for(300ms);
                 return true;
             }},
            {Str::boot_mic, [] { std::this_thread::sleep_for(300ms); return true; }},
        };
        tonus::app::runBoot(steps, boot);
    }).detach();

    if (opt.screen == "home") tonus::ui::show(tonus::ui::Screen::Home);
    else if (opt.screen == "diag") tonus::ui::show(tonus::ui::Screen::Diagnostics);

    const uint32_t start = SDL_GetTicks();
    for (;;) {
        const uint32_t wait = lv_timer_handler();
        if (!opt.screenshot.empty() && SDL_GetTicks() - start >= opt.atMs) {
            lv_refr_now(disp);
            const bool ok = saveScreenshot(opt.screenshot);
            std::printf("[sim] screenshot %s: %s\n", ok ? "saved" : "FAILED", opt.screenshot.c_str());
            return ok ? 0 : 1;
        }
        SDL_Delay(std::min<uint32_t>(std::max<uint32_t>(wait, 1), 10));
    }
}

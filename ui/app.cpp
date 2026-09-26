#include "app.hpp"

#include "i18n/i18n.hpp"
#include "screens/screens.hpp"
#include "theme.hpp"

namespace tonus::ui {
namespace {
DeviceHal* g_hal = nullptr;
const app::BootStatus* g_boot = nullptr;
}  // namespace

void start(DeviceHal& hal, const app::BootStatus& boot) {
    g_hal = &hal;
    g_boot = &boot;
    i18n::setLang(static_cast<i18n::Lang>(hal.loadSetting(kSettingLang, 0)));
    setTheme(static_cast<ThemeId>(hal.loadSetting(kSettingTheme, 0)));
    show(Screen::Splash);
}

void show(Screen screen) {
    lv_obj_t* scr = lv_obj_create(nullptr);
    lv_obj_set_scrollable(scr, false);
    lv_obj_set_style_bg_color(scr, color(Token::bg), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(scr, color(Token::text), 0);
    lv_obj_set_style_text_font(scr, font::body24(), 0);

    switch (screen) {
        case Screen::Splash: buildSplash(scr); break;
        case Screen::Home: buildHome(scr); break;
        case Screen::Diagnostics: buildDiagnostics(scr); break;
    }
    lv_screen_load_anim(scr, LV_SCREEN_LOAD_ANIM_NONE, 0, 0, true);
}

DeviceHal& hal() { return *g_hal; }
const app::BootStatus& bootStatus() { return *g_boot; }

}  // namespace tonus::ui

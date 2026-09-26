// Hidden diagnostics screen: system info, touch test, audio out/in, language and theme toggles.

#include <cmath>
#include <string>

#include "app.hpp"
#include "audio/chiptune.hpp"
#include "i18n/i18n.hpp"
#include "screens/screens.hpp"
#include "theme.hpp"
#include "widgets/pixel_box.hpp"
#include "widgets/segment_bar.hpp"

namespace tonus::ui {
namespace {

using i18n::Str;

struct Diag {
    lv_obj_t* info[6] = {};
    lv_obj_t* touchArea = nullptr;
    lv_obj_t* crossH = nullptr;
    lv_obj_t* crossV = nullptr;
    lv_obj_t* touchLabel = nullptr;
    lv_obj_t* mic = nullptr;
    lv_timer_t* fastTimer = nullptr;
    lv_timer_t* slowTimer = nullptr;
    lv_display_t* display = nullptr;
    uint32_t frames = 0;
    uint32_t fps = 0;
};

std::string kb(size_t bytes) { return std::to_string(bytes / 1024); }

lv_obj_t* infoLine(lv_obj_t* parent, int row) {
    lv_obj_t* l = lv_label_create(parent);
    lv_obj_set_style_text_font(l, font::body24(), 0);
    lv_obj_set_style_text_color(l, color(Token::text), 0);
    lv_obj_set_pos(l, 2 * kArtPx, row * 9 * kArtPx);
    return l;
}

void refreshInfo(Diag* d) {
    const DeviceInfo info = hal().info();
    const auto orUnknown = [](const std::string& s) { return s.empty() ? std::string(i18n::tr(Str::common_unknown)) : s; };
    lv_label_set_text(d->info[0], i18n::format(Str::diag_chip, {{"value", orUnknown(info.chipRevision)}}).c_str());
    lv_label_set_text(d->info[1], i18n::format(Str::diag_idf, {{"value", orUnknown(info.idfVersion)}}).c_str());
    lv_label_set_text(d->info[2], i18n::format(Str::diag_app, {{"value", orUnknown(info.appVersion)}}).c_str());
    lv_label_set_text(d->info[3], i18n::format(Str::diag_panel, {{"value", orUnknown(info.panel)}}).c_str());
    lv_label_set_text(d->info[4], i18n::format(Str::diag_heap, {{"internal", kb(hal().freeInternalHeap())},
                                                                 {"psram", kb(hal().freePsram())}}).c_str());
    lv_label_set_text(d->info[5], i18n::format(Str::diag_fps, {{"value", std::to_string(d->fps)}}).c_str());
}

void onSlowTick(lv_timer_t* t) {
    auto* d = static_cast<Diag*>(lv_timer_get_user_data(t));
    d->fps = d->frames;
    d->frames = 0;
    refreshInfo(d);
}

void onFastTick(lv_timer_t* t) {
    auto* d = static_cast<Diag*>(lv_timer_get_user_data(t));
    // Map RMS to segments on a rough dB scale (-60..0 dBFS) so quiet signals still move the meter.
    const float rms = hal().micLevel();
    const float db = rms > 1e-6f ? 20.0f * std::log10(rms) : -120.0f;
    const float norm = (db + 60.0f) / 60.0f;
    setSegmentsLit(d->mic, static_cast<int>(std::lround(norm * segmentCount(d->mic))));
}

void onRefrReady(lv_event_t* e) { ++static_cast<Diag*>(lv_event_get_user_data(e))->frames; }

void onTouch(lv_event_t* e) {
    auto* d = static_cast<Diag*>(lv_event_get_user_data(e));
    lv_point_t p;
    lv_indev_get_point(lv_indev_active(), &p);
    lv_area_t area;
    lv_obj_get_coords(d->touchArea, &area);
    lv_obj_set_y(d->crossH, p.y - area.y1 - kArtPx / 2);
    lv_obj_set_x(d->crossV, p.x - area.x1 - kArtPx / 2);
    lv_obj_set_hidden(d->crossH, false);
    lv_obj_set_hidden(d->crossV, false);
    lv_label_set_text(d->touchLabel,
                      i18n::format(Str::diag_touch, {{"x", std::to_string(p.x)}, {"y", std::to_string(p.y)}}).c_str());
}

void onDelete(lv_event_t* e) {
    auto* d = static_cast<Diag*>(lv_event_get_user_data(e));
    lv_timer_delete(d->fastTimer);
    lv_timer_delete(d->slowTimer);
    lv_display_remove_event_cb_with_user_data(d->display, onRefrReady, d);
    hal().stopTone();
    hal().setMicEnabled(false);
    delete d;
}

void onBack(lv_event_t*) { show(Screen::Home); }
void onTone440(lv_event_t*) { hal().startTone(440.0f); }
void onTone1k(lv_event_t*) { hal().startTone(1000.0f); }
void onToneStop(lv_event_t*) { hal().stopTone(); }
void onJingle(lv_event_t*) { hal().playPcm(audio::renderBootJingle(48000), 48000); }

void onLanguage(lv_event_t*) {
    const int next = (static_cast<int>(i18n::lang()) + 1) % static_cast<int>(i18n::Lang::Count);
    i18n::setLang(static_cast<i18n::Lang>(next));
    hal().saveSetting(kSettingLang, next);
    show(Screen::Diagnostics);
}

void onTheme(lv_event_t*) {
    const int next = (static_cast<int>(theme()) + 1) % static_cast<int>(ThemeId::Count);
    setTheme(static_cast<ThemeId>(next));
    hal().saveSetting(kSettingTheme, next);
    show(Screen::Diagnostics);
}

lv_obj_t* caption(lv_obj_t* parent, Str text, int x, int y) {
    lv_obj_t* l = lv_label_create(parent);
    lv_label_set_text(l, i18n::tr(text));
    lv_obj_set_style_text_font(l, font::display24(), 0);
    lv_obj_set_style_text_color(l, color(Token::text_dim), 0);
    lv_obj_set_pos(l, x, y);
    return l;
}

lv_obj_t* crossLine(lv_obj_t* parent, bool horizontal) {
    lv_obj_t* line = lv_obj_create(parent);
    lv_obj_remove_style_all(line);
    lv_obj_set_clickable(line, false);
    lv_obj_set_style_bg_color(line, color(Token::pink), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    if (horizontal) lv_obj_set_size(line, lv_pct(100), kArtPx);
    else lv_obj_set_size(line, kArtPx, lv_pct(100));
    lv_obj_set_hidden(line, true);
    return line;
}

}  // namespace

void buildDiagnostics(lv_obj_t* scr) {
    auto* d = new Diag;
    d->display = lv_obj_get_display(scr);
    lv_obj_add_event_cb(scr, onDelete, LV_EVENT_DELETE, d);
    lv_display_add_event_cb(d->display, onRefrReady, LV_EVENT_REFR_READY, d);

    // Header
    lv_obj_t* back = pixelButton(scr, i18n::tr(Str::diag_back), Token::yellow, 216, 104);
    lv_obj_set_pos(back, 24, 24);
    lv_obj_add_event_cb(back, onBack, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* title = lv_label_create(scr);
    lv_label_set_text(title, i18n::tr(Str::diag_title));
    lv_obj_set_style_text_font(title, font::display48(), 0);
    lv_obj_set_style_text_color(title, color(Token::text), 0);
    lv_obj_set_pos(title, 272, 48);

    // System info
    lv_obj_t* sys = windowPanel(scr, i18n::tr(Str::diag_system), 24, 152, 608, 312);
    for (int i = 0; i < 6; ++i) d->info[i] = infoLine(sys, i);
    refreshInfo(d);

    // Audio out
    lv_obj_t* out = windowPanel(scr, i18n::tr(Str::diag_audio_out), 648, 152, 608, 184);
    struct Btn { Str text; Token fill; lv_event_cb_t cb; };
    const Btn buttons[] = {{Str::diag_tone_440, Token::cyan, onTone440},
                           {Str::diag_tone_1k, Token::cyan, onTone1k},
                           {Str::diag_tone_stop, Token::pink, onToneStop},
                           {Str::diag_jingle, Token::primary, onJingle}};
    for (int i = 0; i < 4; ++i) {
        lv_obj_t* b = pixelButton(out, i18n::tr(buttons[i].text), buttons[i].fill, 136, 104);
        lv_obj_set_pos(b, i * 144, 0);
        lv_obj_add_event_cb(b, buttons[i].cb, LV_EVENT_CLICKED, nullptr);
    }

    // Mic level
    lv_obj_t* in = windowPanel(scr, i18n::tr(Str::diag_audio_in), 648, 352, 608, 112);
    SegmentBarStyle meter;
    meter.segments = 18;
    meter.segW = 24;
    meter.segH = 32;
    meter.fill = Token::success;
    d->mic = segmentBar(in, meter);
    lv_obj_set_pos(d->mic, 2 * kArtPx, 0);
    hal().setMicEnabled(true);

    // Touch test
    lv_obj_t* touch = windowPanel(scr, i18n::tr(Str::diag_touch_hint), 24, 480, 608, 216);
    d->touchArea = lv_obj_create(touch);
    lv_obj_remove_style_all(d->touchArea);
    lv_obj_set_scrollable(d->touchArea, false);
    lv_obj_set_size(d->touchArea, lv_pct(100), lv_pct(100));
    lv_obj_set_style_bg_color(d->touchArea, color(Token::surface_alt), 0);
    lv_obj_set_style_bg_opa(d->touchArea, LV_OPA_COVER, 0);
    lv_obj_set_clickable(d->touchArea, true);
    lv_obj_add_event_cb(d->touchArea, onTouch, LV_EVENT_PRESSING, d);
    lv_obj_add_event_cb(d->touchArea, onTouch, LV_EVENT_PRESSED, d);
    d->crossH = crossLine(d->touchArea, true);
    d->crossV = crossLine(d->touchArea, false);
    d->touchLabel = lv_label_create(d->touchArea);
    lv_label_set_text(d->touchLabel, i18n::format(Str::diag_touch, {{"x", "-"}, {"y", "-"}}).c_str());
    lv_obj_set_style_text_font(d->touchLabel, font::body24(), 0);
    lv_obj_set_style_text_color(d->touchLabel, color(Token::text_dim), 0);
    lv_obj_align(d->touchLabel, LV_ALIGN_BOTTOM_RIGHT, -2 * kArtPx, -2 * kArtPx);

    // Settings
    lv_obj_t* set = windowPanel(scr, i18n::tr(Str::diag_settings), 648, 480, 608, 216);
    caption(set, Str::diag_lang, 0, 0);
    caption(set, Str::diag_theme, 296, 0);
    lv_obj_t* lang = pixelButton(set, i18n::tr(i18n::lang() == i18n::Lang::en ? Str::lang_en : Str::lang_pt),
                                 Token::pink, 280, 104);
    lv_obj_set_pos(lang, 0, 36);
    lv_obj_add_event_cb(lang, onLanguage, LV_EVENT_CLICKED, nullptr);
    lv_obj_t* th = pixelButton(set, i18n::tr(theme() == ThemeId::Stage ? Str::theme_stage : Str::theme_bedroom),
                               Token::primary, 280, 104);
    lv_obj_set_pos(th, 296, 36);
    lv_obj_add_event_cb(th, onTheme, LV_EVENT_CLICKED, nullptr);

    d->fastTimer = lv_timer_create(onFastTick, 50, d);
    d->slowTimer = lv_timer_create(onSlowTick, 1000, d);
}

}  // namespace tonus::ui

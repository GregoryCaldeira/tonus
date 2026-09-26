// Splash / loading screen (docs/DESIGN_GUIDELINES.md, "Splash").
// Stars + lightning bolts + the TONUS wordmark dropping in, a chrome glint, the tagline, a
// segmented boot-progress bar with the current boot step, then a blinking "TAP TO START".
// Long-press the logo for 1 s (after boot) to open Diagnostics.

#include <vector>

#include "app.hpp"
#include "art/sprites.hpp"
#include "art/wordmark.hpp"
#include "i18n/i18n.hpp"
#include "screens/screens.hpp"
#include "theme.hpp"
#include "version.hpp"
#include "widgets/pixel_art.hpp"
#include "widgets/segment_bar.hpp"

namespace tonus::ui {
namespace {

constexpr int kLogoScale = 10;
constexpr int kLogoY = 128;
constexpr int kTickMs = 40;
constexpr uint32_t kMinSplashMs = 2500;
constexpr uint32_t kLongPressMs = 1000;
constexpr int kStars = 72;

struct Splash {
    lv_obj_t* scr = nullptr;
    PixelArt* logo = nullptr;
    PixelArt* bolts[2] = {};
    std::vector<lv_obj_t*> stars;
    std::vector<PixelArt*> sparkles;
    lv_obj_t* bar = nullptr;
    lv_obj_t* status = nullptr;
    lv_obj_t* tap = nullptr;
    lv_timer_t* timer = nullptr;

    uint32_t startMs = 0;
    uint32_t tick = 0;
    uint32_t rng = 0x70A5u;
    int glint = -1;
    int glintWait = 25;
    int shownLabel = -2;
    bool ready = false;
    bool reduceMotion = false;
    uint32_t pressStartMs = 0;
    bool longPressFired = false;

    uint32_t random(uint32_t n) {
        rng = rng * 1103515245u + 12345u;
        return (rng >> 16) % n;
    }
};

const std::vector<Token> kWordmarkPalette = {
    Token::bg,         // kWmClear (unused, transparent)
    Token::outline,    // kWmOutline
    Token::chrome_hi,  // kWmChromeHi
    Token::chrome_lo,  // kWmChromeLo
    Token::sparkle,    // kWmHighlight
    Token::pink,       // kWmShadowNear
    Token::primary,    // kWmShadowFar
    Token::sparkle,    // kWmGlint
};

art::WordmarkOptions glintAt(int pos) {
    art::WordmarkOptions o;
    o.glint = pos;
    return o;
}

int snap(int v) { return (v / kArtPx) * kArtPx; }

void logoDropExec(void* obj, int32_t y) { lv_obj_set_y(static_cast<lv_obj_t*>(obj), snap(y)); }

void goHome(Splash* s) {
    if (s->ready && !s->longPressFired) show(Screen::Home);
}

void onScreenClicked(lv_event_t* e) { goHome(static_cast<Splash*>(lv_event_get_user_data(e))); }

void onLogoEvent(lv_event_t* e) {
    auto* s = static_cast<Splash*>(lv_event_get_user_data(e));
    switch (lv_event_get_code(e)) {
        case LV_EVENT_PRESSED:
            s->pressStartMs = lv_tick_get();
            s->longPressFired = false;
            break;
        case LV_EVENT_PRESSING:
            if (!s->longPressFired && bootStatus().finished && lv_tick_elaps(s->pressStartMs) >= kLongPressMs) {
                s->longPressFired = true;
                show(Screen::Diagnostics);
            }
            break;
        case LV_EVENT_CLICKED: goHome(s); break;
        default: break;
    }
}

void updateProgress(Splash* s) {
    const app::BootStatus& boot = bootStatus();
    const int total = boot.total.load() > 0 ? boot.total.load() : 1;
    const int segs = segmentCount(s->bar);
    const int target = boot.completed.load() * segs / total;
    // Fill at most one segment every other tick so fast boot steps still read as progress.
    if (segmentsLit(s->bar) < target && (s->tick % 2 == 0 || s->reduceMotion))
        setSegmentsLit(s->bar, s->reduceMotion ? target : segmentsLit(s->bar) + 1);

    // Show the running step; once boot is over, show the first failure instead (if any).
    constexpr int kShowFailure = -3;
    const int failed = boot.failedLabel.load();
    const int label = failed >= 0 && boot.finished ? kShowFailure : boot.label.load();
    if (label != s->shownLabel) {
        s->shownLabel = label;
        if (label == kShowFailure) {
            const std::string msg = i18n::format(i18n::Str::boot_error,
                                                 {{"step", i18n::tr(static_cast<i18n::Str>(failed))}});
            lv_label_set_text(s->status, msg.c_str());
            lv_obj_set_style_text_color(s->status, color(Token::late), 0);
        } else {
            lv_label_set_text(s->status, label >= 0 ? i18n::tr(static_cast<i18n::Str>(label)) : "");
        }
    }

    if (!s->ready && boot.finished && segmentsLit(s->bar) >= segs && lv_tick_elaps(s->startMs) >= kMinSplashMs) {
        s->ready = true;
        // Keep a failure message visible under the tap prompt; otherwise swap it out.
        if (failed < 0) lv_obj_set_hidden(s->status, true);
        lv_obj_set_hidden(s->tap, false);
    }
}

void animate(Splash* s) {
    if (s->reduceMotion) return;

    // Chrome glint: sweep every ~3 s, 80 ms per step.
    if (s->tick % 2 == 0) {
        if (s->glint >= 0) {
            s->glint += 3;
            if (s->glint > art::wordmarkGlintEnd()) {
                s->glint = -1;
                s->glintWait = 37;
            }
            s->logo->setBitmap(art::composeWordmark(glintAt(s->glint)));
        } else if (--s->glintWait <= 0) {
            s->glint = 0;
        }
    }

    // Star twinkle: a few stars swap between dim and bright tones.
    if (s->tick % 6 == 0)
        for (int i = 0; i < 3; ++i) {
            lv_obj_t* star = s->stars[s->random(static_cast<uint32_t>(s->stars.size()))];
            const bool bright = lv_color_eq(lv_obj_get_style_bg_color(star, LV_PART_MAIN), color(Token::text_dim));
            lv_obj_set_style_bg_color(star, color(bright ? Token::bg_grid : Token::text_dim), 0);
        }

    // Sparkles blink slowly, bolts flicker at ~8 fps.
    if (s->tick % 10 == 0)
        for (PixelArt* sp : s->sparkles)
            if (s->random(3) == 0) lv_obj_set_hidden(sp->obj(), !lv_obj_is_hidden(sp->obj()));
    if (s->tick % 3 == 0)
        for (PixelArt* bolt : s->bolts) lv_obj_set_hidden(bolt->obj(), s->random(5) == 0);

    // "TAP TO START" blinks at 2 Hz.
    if (s->ready) lv_obj_set_style_opa(s->tap, (s->tick / 6) % 2 ? LV_OPA_TRANSP : LV_OPA_COVER, 0);
}

void onTick(lv_timer_t* t) {
    auto* s = static_cast<Splash*>(lv_timer_get_user_data(t));
    ++s->tick;
    updateProgress(s);
    animate(s);
}

void onDelete(lv_event_t* e) {
    auto* s = static_cast<Splash*>(lv_event_get_user_data(e));
    lv_timer_delete(s->timer);
    delete s;
}

void buildStars(Splash* s) {
    for (int i = 0; i < kStars; ++i) {
        lv_obj_t* star = lv_obj_create(s->scr);
        lv_obj_remove_style_all(star);
        lv_obj_set_clickable(star, false);
        const int size = (s->random(4) == 0 ? 2 : 1) * kArtPx;
        lv_obj_set_size(star, size, size);
        lv_obj_set_pos(star, snap(static_cast<int>(s->random(kScreenW))), snap(static_cast<int>(s->random(kScreenH))));
        lv_obj_set_style_bg_opa(star, LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(star, color(s->random(3) == 0 ? Token::text_dim : Token::bg_grid), 0);
        s->stars.push_back(star);
    }

    struct Pos { int x, y; Token t; };
    const Pos sparkles[] = {{96, 72, Token::sparkle}, {1136, 96, Token::pink}, {176, 612, Token::pink},
                            {1080, 588, Token::sparkle}, {640, 48, Token::yellow}, {56, 380, Token::sparkle}};
    for (const Pos& p : sparkles) {
        PixelArt* sp = PixelArt::create(s->scr, kArtPx, {Token::bg, Token::outline, p.t});
        sp->setBitmap(art::sparkle());
        lv_obj_set_clickable(sp->obj(), false);
        lv_obj_set_pos(sp->obj(), p.x, p.y);
        s->sparkles.push_back(sp);
    }
}

}  // namespace

void buildSplash(lv_obj_t* scr) {
    auto* s = new Splash;
    s->scr = scr;
    s->startMs = lv_tick_get();
    s->reduceMotion = hal().loadSetting(kSettingReduceMotion, 0) != 0;
    lv_obj_add_event_cb(scr, onDelete, LV_EVENT_DELETE, s);
    lv_obj_add_event_cb(scr, onScreenClicked, LV_EVENT_CLICKED, s);

    buildStars(s);

    // Wordmark
    s->logo = PixelArt::create(scr, kLogoScale, kWordmarkPalette);
    s->logo->setBitmap(art::composeWordmark());
    const int logoX = snap((kScreenW - s->logo->width()) / 2);
    lv_obj_set_x(s->logo->obj(), logoX);
    lv_obj_set_clickable(s->logo->obj(), true);
    lv_obj_add_event_cb(s->logo->obj(), onLogoEvent, LV_EVENT_ALL, s);

    // Lightning bolts either side of the logo
    for (int i = 0; i < 2; ++i) {
        s->bolts[i] = PixelArt::create(scr, 6, {Token::bg, Token::outline, Token::yellow});
        s->bolts[i]->setBitmap(art::lightningBolt());
        lv_obj_set_clickable(s->bolts[i]->obj(), false);
        const int bx = i == 0 ? logoX - s->bolts[i]->width() - 24 : logoX + s->logo->width() + 24;
        lv_obj_set_pos(s->bolts[i]->obj(), snap(bx), kLogoY + (s->logo->height() - s->bolts[i]->height()) / 2);
    }

    if (s->reduceMotion) {
        lv_obj_set_y(s->logo->obj(), kLogoY);
    } else {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, s->logo->obj());
        lv_anim_set_exec_cb(&a, logoDropExec);
        lv_anim_set_values(&a, -s->logo->height(), kLogoY);
        lv_anim_set_duration(&a, 450);
        lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
        lv_anim_start(&a);
    }

    // Tagline
    lv_obj_t* tagline = lv_label_create(scr);
    lv_label_set_text(tagline, i18n::tr(i18n::Str::splash_tagline));
    lv_obj_set_style_text_font(tagline, font::display32(), 0);
    lv_obj_set_style_text_color(tagline, color(Token::text), 0);
    lv_obj_align(tagline, LV_ALIGN_TOP_MID, 0, 392);

    // Boot progress
    SegmentBarStyle barStyle;
    s->bar = segmentBar(scr, barStyle);
    lv_obj_align(s->bar, LV_ALIGN_TOP_MID, 0, 480);

    s->status = lv_label_create(scr);
    lv_label_set_text(s->status, "");
    lv_obj_set_style_text_font(s->status, font::body24(), 0);
    lv_obj_set_style_text_color(s->status, color(Token::text_dim), 0);
    lv_obj_align(s->status, LV_ALIGN_TOP_MID, 0, 536);

    s->tap = lv_label_create(scr);
    lv_label_set_text(s->tap, i18n::tr(i18n::Str::splash_tap));
    lv_obj_set_style_text_font(s->tap, font::display32(), 0);
    lv_obj_set_style_text_color(s->tap, color(Token::yellow), 0);
    lv_obj_align(s->tap, LV_ALIGN_TOP_MID, 0, 576);
    lv_obj_set_hidden(s->tap, true);

    lv_obj_t* version = lv_label_create(scr);
    lv_label_set_text(version, i18n::format(i18n::Str::app_version, {{"version", kAppVersion}}).c_str());
    lv_obj_set_style_text_font(version, font::body16(), 0);
    lv_obj_set_style_text_color(version, color(Token::text_dim), 0);
    lv_obj_align(version, LV_ALIGN_BOTTOM_RIGHT, -24, -24);

    s->timer = lv_timer_create(onTick, kTickMs, s);
}

}  // namespace tonus::ui

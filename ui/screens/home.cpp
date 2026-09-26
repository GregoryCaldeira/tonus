// Placeholder Home ("Backstage") until Phase 1 brings the character and the nav rail.

#include "app.hpp"
#include "art/wordmark.hpp"
#include "i18n/i18n.hpp"
#include "screens/screens.hpp"
#include "theme.hpp"
#include "widgets/pixel_art.hpp"
#include "widgets/pixel_box.hpp"

namespace tonus::ui {
namespace {

void onDiagnostics(lv_event_t*) { show(Screen::Diagnostics); }

}  // namespace

void buildHome(lv_obj_t* scr) {
    PixelArt* logo = PixelArt::create(scr, kArtPx,
                                      {Token::bg, Token::outline, Token::chrome_hi, Token::chrome_lo, Token::sparkle,
                                       Token::pink, Token::primary, Token::sparkle});
    logo->setBitmap(art::composeWordmark());
    lv_obj_align(logo->obj(), LV_ALIGN_TOP_MID, 0, 40);

    constexpr int kW = 832, kH = 400;
    lv_obj_t* body = windowPanel(scr, i18n::tr(i18n::Str::home_title), (kScreenW - kW) / 2, 176, kW, kH);

    lv_obj_t* text = lv_label_create(body);
    lv_label_set_text(text, i18n::tr(i18n::Str::home_soon));
    lv_label_set_long_mode(text, LV_LABEL_LONG_MODE_WRAP);
    lv_obj_set_width(text, lv_pct(100));
    lv_obj_set_style_text_font(text, font::body32(), 0);
    lv_obj_set_style_text_color(text, color(Token::text), 0);
    lv_obj_set_style_text_line_space(text, 2 * kArtPx, 0);
    lv_obj_align(text, LV_ALIGN_TOP_LEFT, 2 * kArtPx, 2 * kArtPx);

    lv_obj_t* diag = pixelButton(body, i18n::tr(i18n::Str::home_diagnostics), Token::cyan, 360, 104);
    lv_obj_align(diag, LV_ALIGN_BOTTOM_MID, 0, -kArtPx);
    lv_obj_add_event_cb(diag, onDiagnostics, LV_EVENT_CLICKED, nullptr);
}

}  // namespace tonus::ui

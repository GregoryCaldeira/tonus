#include "widgets/pixel_box.hpp"

namespace tonus::ui {
namespace {

constexpr int a = kArtPx;

void fillRect(lv_layer_t* layer, int x1, int y1, int x2, int y2, lv_color_t c) {
    if (x2 < x1 || y2 < y1) return;
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = c;
    dsc.bg_opa = LV_OPA_COVER;
    dsc.radius = 0;
    dsc.border_width = 0;
    lv_area_t area = {static_cast<int32_t>(x1), static_cast<int32_t>(y1), static_cast<int32_t>(x2),
                      static_cast<int32_t>(y2)};
    lv_draw_rect(layer, &dsc, &area);
}

// A rectangle with each corner notched by one art-px (the "stepped" corner).
void steppedRect(lv_layer_t* layer, int x1, int y1, int x2, int y2, int notch, lv_color_t c) {
    fillRect(layer, x1 + notch, y1, x2 - notch, y2, c);
    fillRect(layer, x1, y1 + notch, x2, y2 - notch, c);
}

void drawBox(lv_layer_t* layer, const lv_area_t& coords, const PixelBoxStyle& s, bool pressed) {
    const int shift = (s.pressable && pressed) ? kShadowOffset : 0;
    const int x1 = coords.x1 + shift, y1 = coords.y1 + shift;
    const int x2 = coords.x2 - kShadowOffset + shift, y2 = coords.y2 - kShadowOffset + shift;

    if (s.shadow && shift == 0)
        steppedRect(layer, x1 + kShadowOffset, y1 + kShadowOffset, x2 + kShadowOffset, y2 + kShadowOffset, a,
                    color(Token::outline));
    steppedRect(layer, x1, y1, x2, y2, a, color(s.outline));
    steppedRect(layer, x1 + a, y1 + a, x2 - a, y2 - a, a, color(s.fill));
    if (s.dashes) {
        // Short highlight dash top-left and a dot after it, as in the button reference.
        const lv_color_t hi = lighter(s.fill);
        fillRect(layer, x1 + 3 * a, y1 + 2 * a, x1 + 7 * a - 1, y1 + 3 * a - 1, hi);
        fillRect(layer, x1 + 8 * a, y1 + 2 * a, x1 + 9 * a - 1, y1 + 3 * a - 1, hi);
        fillRect(layer, x2 - 7 * a + 1, y2 - 3 * a + 1, x2 - 3 * a, y2 - 2 * a, darker(s.fill));
    }
}

void onDraw(lv_event_t* e) {
    lv_obj_t* obj = lv_event_get_target_obj(e);
    auto* style = static_cast<PixelBoxStyle*>(lv_event_get_user_data(e));
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);
    drawBox(lv_event_get_layer(e), coords, *style, lv_obj_has_state(obj, LV_STATE_PRESSED));
}

void onDelete(lv_event_t* e) { delete static_cast<PixelBoxStyle*>(lv_event_get_user_data(e)); }

PixelBoxStyle* styleOf(lv_obj_t* obj) {
    const uint32_t n = lv_obj_get_event_count(obj);
    for (uint32_t i = 0; i < n; ++i) {
        lv_event_dsc_t* dsc = lv_obj_get_event_dsc(obj, i);
        if (lv_event_dsc_get_cb(dsc) == onDraw) return static_cast<PixelBoxStyle*>(lv_event_dsc_get_user_data(dsc));
    }
    return nullptr;
}

void onButtonState(lv_event_t* e) {
    lv_obj_t* btn = lv_event_get_target_obj(e);
    lv_obj_t* label = lv_obj_get_child(btn, 0);
    if (!label) return;
    const int32_t d = lv_obj_has_state(btn, LV_STATE_PRESSED) ? kShadowOffset : 0;
    lv_obj_set_style_translate_x(label, d, 0);
    lv_obj_set_style_translate_y(label, d, 0);
}

}  // namespace

void makePixelBox(lv_obj_t* obj, const PixelBoxStyle& style) {
    lv_obj_remove_style_all(obj);
    lv_obj_set_scrollable(obj, false);
    auto* copy = new PixelBoxStyle(style);
    lv_obj_add_event_cb(obj, onDraw, LV_EVENT_DRAW_MAIN, copy);
    lv_obj_add_event_cb(obj, onDelete, LV_EVENT_DELETE, copy);
}

void setPixelBoxFill(lv_obj_t* obj, Token fill) {
    if (PixelBoxStyle* s = styleOf(obj)) {
        s->fill = fill;
        lv_obj_invalidate(obj);
    }
}

lv_obj_t* pixelButton(lv_obj_t* parent, const char* text, Token fill, int w, int h) {
    lv_obj_t* btn = lv_button_create(parent);
    PixelBoxStyle style;
    style.fill = fill;
    style.dashes = true;
    style.pressable = true;
    makePixelBox(btn, style);
    lv_obj_set_size(btn, w, h);
    lv_obj_add_event_cb(btn, onButtonState, LV_EVENT_PRESSED, nullptr);
    lv_obj_add_event_cb(btn, onButtonState, LV_EVENT_RELEASED, nullptr);
    lv_obj_add_event_cb(btn, onButtonState, LV_EVENT_PRESS_LOST, nullptr);

    lv_obj_t* label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font::display24(), 0);
    lv_obj_set_style_text_color(label, color(Token::on_fill), 0);
    lv_obj_set_style_text_letter_space(label, a / 2, 0);
    // Centre on the face, not on the face + shadow.
    lv_obj_align(label, LV_ALIGN_CENTER, -kShadowOffset / 2, -kShadowOffset / 2);
    return btn;
}

void setPixelButtonText(lv_obj_t* button, const char* text) {
    if (lv_obj_t* label = lv_obj_get_child(button, 0)) lv_label_set_text(label, text);
}

lv_obj_t* windowPanel(lv_obj_t* parent, const char* title, int x, int y, int w, int h) {
    lv_obj_t* win = lv_obj_create(parent);
    PixelBoxStyle style;
    style.fill = Token::surface;
    makePixelBox(win, style);
    lv_obj_set_pos(win, x, y);
    lv_obj_set_size(win, w, h);

    const int innerW = w - kShadowOffset - 2 * a;
    constexpr int kTitleH = 12 * a;

    lv_obj_t* bar = lv_obj_create(win);
    lv_obj_remove_style_all(bar);
    lv_obj_set_pos(bar, a, a);
    lv_obj_set_size(bar, innerW, kTitleH);
    lv_obj_set_style_bg_color(bar, color(Token::titlebar), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_side(bar, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_width(bar, a, 0);
    lv_obj_set_style_border_color(bar, color(Token::outline), 0);

    lv_obj_t* label = lv_label_create(bar);
    lv_label_set_text(label, title);
    lv_obj_set_style_text_font(label, font::display24(), 0);
    lv_obj_set_style_text_color(label, color(Token::on_fill), 0);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 4 * a, -a / 2);

    // "○○○" window controls: outlined squares.
    for (int i = 0; i < 3; ++i) {
        lv_obj_t* dot = lv_obj_create(bar);
        lv_obj_remove_style_all(dot);
        lv_obj_set_size(dot, 3 * a, 3 * a);
        lv_obj_set_style_border_width(dot, a, 0);
        lv_obj_set_style_border_color(dot, color(Token::outline), 0);
        lv_obj_align(dot, LV_ALIGN_RIGHT_MID, -(4 * a + i * 5 * a), -a / 2);
    }

    lv_obj_t* body = lv_obj_create(win);
    lv_obj_remove_style_all(body);
    lv_obj_set_scrollable(body, false);
    lv_obj_set_pos(body, 2 * a, kTitleH + 2 * a);
    lv_obj_set_size(body, innerW - 2 * a, h - kShadowOffset - kTitleH - 4 * a);
    return body;
}

}  // namespace tonus::ui

#include "theme.hpp"

LV_FONT_DECLARE(tonus_font_display_48)
LV_FONT_DECLARE(tonus_font_display_32)
LV_FONT_DECLARE(tonus_font_display_24)
LV_FONT_DECLARE(tonus_font_body_32)
LV_FONT_DECLARE(tonus_font_body_24)
LV_FONT_DECLARE(tonus_font_body_16)

namespace tonus::ui {
namespace {

constexpr int kTokens = static_cast<int>(Token::Count);

// Order must match enum class Token.
constexpr uint32_t kStage[kTokens] = {
    0x140C2B,  // bg
    0x1F1440,  // bg_grid
    0x231A45,  // surface
    0x2F2360,  // surface_alt
    0x9B5CFF,  // titlebar
    0x07040F,  // outline
    0xFFF4E0,  // text
    0xB7A8E0,  // text_dim
    0x1B1B2F,  // on_fill
    0x9B5CFF,  // primary
    0xFF6FAE,  // pink
    0x5EC8FF,  // cyan
    0xFFC857,  // yellow
    0x6EE7A0,  // success
    0x5EC8FF,  // early
    0xFF8A5C,  // late
    0xFF4D6D,  // danger
    0xFFF4E0,  // chrome_hi
    0xC9B8F0,  // chrome_lo
    0xFFFFFF,  // sparkle
};

constexpr uint32_t kBedroom[kTokens] = {
    0xFFB3D6,  // bg
    0xFFCCE4,  // bg_grid
    0xFFF4E0,  // surface
    0xFFE3EF,  // surface_alt
    0xFFD66B,  // titlebar
    0x1B1B2F,  // outline
    0x1B1B2F,  // text
    0x6B5A7A,  // text_dim
    0x1B1B2F,  // on_fill
    0x9B5CFF,  // primary
    0xFF6FAE,  // pink
    0x5EC8FF,  // cyan
    0xFFC857,  // yellow
    0x2FA866,  // success
    0x2B8FD6,  // early
    0xE0602F,  // late
    0xD93355,  // danger
    0xFFFFFF,  // chrome_hi
    0xE7D6FF,  // chrome_lo
    0xFFFFFF,  // sparkle
};

ThemeId g_theme = ThemeId::Stage;

}  // namespace

void setTheme(ThemeId id) {
    if (id < ThemeId::Count) g_theme = id;
}

ThemeId theme() { return g_theme; }

uint32_t rgb(Token t) {
    const uint32_t* table = g_theme == ThemeId::Bedroom ? kBedroom : kStage;
    return table[static_cast<int>(t)];
}

lv_color_t color(Token t) { return lv_color_hex(rgb(t)); }
lv_color_t lighter(Token t) { return lv_color_lighten(color(t), LV_OPA_40); }
lv_color_t darker(Token t) { return lv_color_darken(color(t), LV_OPA_30); }

namespace font {
const lv_font_t* display48() { return &tonus_font_display_48; }
const lv_font_t* display32() { return &tonus_font_display_32; }
const lv_font_t* display24() { return &tonus_font_display_24; }
const lv_font_t* body32() { return &tonus_font_body_32; }
const lv_font_t* body24() { return &tonus_font_body_24; }
const lv_font_t* body16() { return &tonus_font_body_16; }
}  // namespace font

}  // namespace tonus::ui

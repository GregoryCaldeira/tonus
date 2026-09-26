#pragma once
// Pixel-honest boxes: stepped corners, 1 art-px outline, optional highlight dashes and a hard
// 2 art-px drop shadow (docs/DESIGN_GUIDELINES.md §8). Drawn with plain rectangles, no radius.

#include "lvgl.h"
#include "theme.hpp"

namespace tonus::ui {

struct PixelBoxStyle {
    Token fill = Token::surface;
    Token outline = Token::outline;
    bool shadow = true;      // hard shadow bottom-right; hidden while pressed
    bool dashes = false;     // highlight dashes top-left (button style)
    bool pressable = false;  // shift the face by the shadow offset while pressed
};

inline constexpr int kShadowOffset = 2 * kArtPx;

// Makes `obj` draw itself as a pixel box. The object's size includes the shadow offset.
// Clears the default LVGL styles of obj.
void makePixelBox(lv_obj_t* obj, const PixelBoxStyle& style);
void setPixelBoxFill(lv_obj_t* obj, Token fill);

// A pressable pixel button with a centred uppercase label. Size includes the shadow.
lv_obj_t* pixelButton(lv_obj_t* parent, const char* text, Token fill, int w, int h);
void setPixelButtonText(lv_obj_t* button, const char* text);

// A window panel (title bar + three "○" controls + body). Returns the body container.
lv_obj_t* windowPanel(lv_obj_t* parent, const char* title, int x, int y, int w, int h);

}  // namespace tonus::ui

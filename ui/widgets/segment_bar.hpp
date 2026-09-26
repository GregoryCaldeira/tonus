#pragma once
// Segmented bar (XP-bar style): N outlined square segments, filled left to right.

#include "lvgl.h"
#include "theme.hpp"

namespace tonus::ui {

struct SegmentBarStyle {
    int segments = 20;
    int segW = 24;
    int segH = 32;
    int gap = 2 * kArtPx;
    Token fill = Token::pink;
    Token empty = Token::surface_alt;
};

// Returns the container; its width is segments*segW + (segments-1)*gap.
lv_obj_t* segmentBar(lv_obj_t* parent, const SegmentBarStyle& style);
// Lights the first `lit` segments.
void setSegmentsLit(lv_obj_t* bar, int lit);
int segmentsLit(lv_obj_t* bar);
int segmentCount(lv_obj_t* bar);

}  // namespace tonus::ui

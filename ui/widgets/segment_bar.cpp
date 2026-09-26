#include "widgets/segment_bar.hpp"

namespace tonus::ui {
namespace {

struct BarState {
    SegmentBarStyle style;
    int lit = -1;
};

void onDelete(lv_event_t* e) { delete static_cast<BarState*>(lv_event_get_user_data(e)); }

BarState* stateOf(lv_obj_t* bar) { return static_cast<BarState*>(lv_obj_get_user_data(bar)); }

}  // namespace

lv_obj_t* segmentBar(lv_obj_t* parent, const SegmentBarStyle& style) {
    lv_obj_t* bar = lv_obj_create(parent);
    lv_obj_remove_style_all(bar);
    lv_obj_set_scrollable(bar, false);
    lv_obj_set_clickable(bar, false);
    lv_obj_set_size(bar, style.segments * style.segW + (style.segments - 1) * style.gap, style.segH);

    auto* state = new BarState{style};
    lv_obj_set_user_data(bar, state);
    lv_obj_add_event_cb(bar, onDelete, LV_EVENT_DELETE, state);

    for (int i = 0; i < style.segments; ++i) {
        lv_obj_t* seg = lv_obj_create(bar);
        lv_obj_remove_style_all(seg);
        lv_obj_set_clickable(seg, false);
        lv_obj_set_pos(seg, i * (style.segW + style.gap), 0);
        lv_obj_set_size(seg, style.segW, style.segH);
        lv_obj_set_style_bg_opa(seg, LV_OPA_COVER, 0);
        lv_obj_set_style_border_width(seg, kArtPx, 0);
        lv_obj_set_style_border_color(seg, color(Token::outline), 0);
    }
    setSegmentsLit(bar, 0);
    return bar;
}

void setSegmentsLit(lv_obj_t* bar, int lit) {
    BarState* s = stateOf(bar);
    if (!s) return;
    if (lit < 0) lit = 0;
    if (lit > s->style.segments) lit = s->style.segments;
    if (lit == s->lit) return;
    s->lit = lit;
    for (int i = 0; i < s->style.segments; ++i)
        lv_obj_set_style_bg_color(lv_obj_get_child(bar, i), color(i < lit ? s->style.fill : s->style.empty), 0);
}

int segmentsLit(lv_obj_t* bar) {
    BarState* s = stateOf(bar);
    return s ? s->lit : 0;
}

int segmentCount(lv_obj_t* bar) {
    BarState* s = stateOf(bar);
    return s ? s->style.segments : 0;
}

}  // namespace tonus::ui

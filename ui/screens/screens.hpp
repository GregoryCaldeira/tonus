#pragma once

#include "lvgl.h"

namespace tonus::ui {

// Each builder fills a fresh, empty screen object (background already set by show()).
void buildSplash(lv_obj_t* scr);
void buildHome(lv_obj_t* scr);
void buildDiagnostics(lv_obj_t* scr);

}  // namespace tonus::ui

#pragma once
// Shows a palette-indexed bitmap as crisp pixel art: pre-scaled by an integer factor into an
// ARGB8888 buffer, so LVGL never resamples it. Palette entries are theme tokens (index 0 = clear).

#include <vector>

#include "art/indexed_bitmap.hpp"
#include "lvgl.h"
#include "theme.hpp"

namespace tonus::ui {

class PixelArt {
public:
    // Creates the image object; the PixelArt is deleted together with it.
    static PixelArt* create(lv_obj_t* parent, int scale, std::vector<Token> palette);

    void setBitmap(const art::IndexedBitmap& bitmap);
    void setPalette(std::vector<Token> palette);  // re-renders with the current theme
    lv_obj_t* obj() const { return img_; }
    int width() const { return bitmap_.width * scale_; }
    int height() const { return bitmap_.height * scale_; }

private:
    PixelArt(lv_obj_t* parent, int scale, std::vector<Token> palette);
    void render();
    static void onDelete(lv_event_t* e);

    lv_obj_t* img_;
    int scale_;
    std::vector<Token> palette_;
    art::IndexedBitmap bitmap_;
    std::vector<uint32_t> pixels_;
    lv_image_dsc_t dsc_{};
};

}  // namespace tonus::ui

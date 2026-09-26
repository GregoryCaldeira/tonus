#include "widgets/pixel_art.hpp"

#include <algorithm>

namespace tonus::ui {

PixelArt* PixelArt::create(lv_obj_t* parent, int scale, std::vector<Token> palette) {
    return new PixelArt(parent, scale, std::move(palette));
}

PixelArt::PixelArt(lv_obj_t* parent, int scale, std::vector<Token> palette)
    : img_(lv_image_create(parent)), scale_(std::max(1, scale)), palette_(std::move(palette)) {
    lv_obj_set_scrollable(img_, false);
    lv_image_set_antialias(img_, false);
    lv_obj_add_event_cb(img_, onDelete, LV_EVENT_DELETE, this);
}

void PixelArt::onDelete(lv_event_t* e) {
    delete static_cast<PixelArt*>(lv_event_get_user_data(e));
}

void PixelArt::setBitmap(const art::IndexedBitmap& bitmap) {
    bitmap_ = bitmap;
    render();
}

void PixelArt::setPalette(std::vector<Token> palette) {
    palette_ = std::move(palette);
    render();
}

void PixelArt::render() {
    const int w = bitmap_.width * scale_;
    const int h = bitmap_.height * scale_;
    if (w <= 0 || h <= 0) return;

    // Resolve the palette once per render.
    std::vector<uint32_t> argb(palette_.size() + 1, 0);
    for (size_t i = 1; i < palette_.size(); ++i) argb[i] = 0xFF000000u | rgb(palette_[i]);

    // The buffer keeps its address while the size is unchanged, so LVGL can keep drawing from it.
    // (The LVGL image cache is disabled: CONFIG_LV_CACHE_DEF_SIZE=0 / LV_CACHE_DEF_SIZE 0.)
    const bool resized = pixels_.size() != static_cast<size_t>(w) * h;
    pixels_.assign(static_cast<size_t>(w) * h, 0);
    for (int y = 0; y < bitmap_.height; ++y) {
        uint32_t* row = &pixels_[static_cast<size_t>(y) * scale_ * w];
        for (int x = 0; x < bitmap_.width; ++x) {
            const uint8_t idx = bitmap_.at(x, y);
            const uint32_t c = idx < argb.size() ? argb[idx] : 0;
            std::fill(row + x * scale_, row + (x + 1) * scale_, c);
        }
        for (int r = 1; r < scale_; ++r) std::copy(row, row + w, row + static_cast<size_t>(r) * w);
    }

    if (resized) {
        dsc_.header.magic = LV_IMAGE_HEADER_MAGIC;
        dsc_.header.cf = LV_COLOR_FORMAT_ARGB8888;
        dsc_.header.w = static_cast<uint32_t>(w);
        dsc_.header.h = static_cast<uint32_t>(h);
        dsc_.header.stride = static_cast<uint32_t>(w * 4);
        dsc_.data_size = static_cast<uint32_t>(pixels_.size() * 4);
        dsc_.data = reinterpret_cast<const uint8_t*>(pixels_.data());
        lv_image_set_src(img_, &dsc_);
    }
    lv_obj_invalidate(img_);
}

}  // namespace tonus::ui

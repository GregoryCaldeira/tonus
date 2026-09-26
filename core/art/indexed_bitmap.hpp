#pragma once
// Palette-indexed pixel art buffer. Index 0 is always transparent; what the other indices mean
// is up to the producer (see wordmark.hpp / sprites.hpp). The UI maps indices to theme tokens.

#include <cstdint>
#include <vector>

namespace tonus::art {

struct IndexedBitmap {
    int width = 0;
    int height = 0;
    std::vector<uint8_t> px;  // row-major, width * height

    IndexedBitmap() = default;
    IndexedBitmap(int w, int h) : width(w), height(h), px(static_cast<size_t>(w) * h, 0) {}

    bool inside(int x, int y) const { return x >= 0 && y >= 0 && x < width && y < height; }
    uint8_t at(int x, int y) const { return inside(x, y) ? px[static_cast<size_t>(y) * width + x] : 0; }
    void set(int x, int y, uint8_t v) {
        if (inside(x, y)) px[static_cast<size_t>(y) * width + x] = v;
    }
};

// A 1-bit mask authored as ASCII rows: '#' is filled, anything else is empty.
struct AsciiMask {
    const char* const* rows;
    int height;
    int width() const;
    bool filled(int x, int y) const;
};

// Paints `mask` at (ox, oy): first its 8-neighbour dilation in `outline`, then the mask in `fill`.
// `fill` may be 0 to paint only the outline ring.
void stampWithOutline(IndexedBitmap& dst, const AsciiMask& mask, int ox, int oy, uint8_t outline,
                      uint8_t fill);

// A standalone sprite: the mask with a 1 art-px outline around it (canvas grows by 2 each way).
IndexedBitmap composeOutlinedSprite(const AsciiMask& mask, uint8_t outline, uint8_t fill);

// A sprite with no outline (canvas equals the mask size).
IndexedBitmap composeFlatSprite(const AsciiMask& mask, uint8_t fill);

}  // namespace tonus::art

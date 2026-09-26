#include "art/indexed_bitmap.hpp"

#include <cstring>

namespace tonus::art {

int AsciiMask::width() const {
    int w = 0;
    for (int y = 0; y < height; ++y) {
        const int len = static_cast<int>(std::strlen(rows[y]));
        if (len > w) w = len;
    }
    return w;
}

bool AsciiMask::filled(int x, int y) const {
    if (y < 0 || y >= height || x < 0) return false;
    const char* row = rows[y];
    const int len = static_cast<int>(std::strlen(row));
    return x < len && row[x] == '#';
}

void stampWithOutline(IndexedBitmap& dst, const AsciiMask& mask, int ox, int oy, uint8_t outline,
                      uint8_t fill) {
    const int w = mask.width();
    for (int y = -1; y <= mask.height; ++y) {
        for (int x = -1; x <= w; ++x) {
            bool near = false;
            for (int dy = -1; dy <= 1 && !near; ++dy)
                for (int dx = -1; dx <= 1 && !near; ++dx) near = mask.filled(x + dx, y + dy);
            if (near) dst.set(ox + x, oy + y, outline);
        }
    }
    if (fill == 0) return;
    for (int y = 0; y < mask.height; ++y)
        for (int x = 0; x < w; ++x)
            if (mask.filled(x, y)) dst.set(ox + x, oy + y, fill);
}

IndexedBitmap composeOutlinedSprite(const AsciiMask& mask, uint8_t outline, uint8_t fill) {
    IndexedBitmap bmp(mask.width() + 2, mask.height + 2);
    stampWithOutline(bmp, mask, 1, 1, outline, fill);
    return bmp;
}

IndexedBitmap composeFlatSprite(const AsciiMask& mask, uint8_t fill) {
    IndexedBitmap bmp(mask.width(), mask.height);
    for (int y = 0; y < mask.height; ++y)
        for (int x = 0; x < bmp.width; ++x)
            if (mask.filled(x, y)) bmp.set(x, y, fill);
    return bmp;
}

}  // namespace tonus::art

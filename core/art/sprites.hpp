#pragma once
// Small decorative sprites for the splash screen.

#include "art/indexed_bitmap.hpp"

namespace tonus::art {

// Palette indices shared by the sprites below.
enum SpritePx : uint8_t { kSpClear = 0, kSpOutline, kSpFill, kSpCount };

IndexedBitmap lightningBolt();  // outlined, fill → token yellow
IndexedBitmap sparkle();        // 5×5 four-point star, no outline, fill → token sparkle / pink

}  // namespace tonus::art

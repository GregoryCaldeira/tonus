#pragma once
// The TONUS wordmark: original spiky "rock pixel" lettering with a chrome face and a solid,
// two-colour extruded shadow (see docs/DESIGN_GUIDELINES.md, "Logo & wordmark").

#include "art/indexed_bitmap.hpp"

namespace tonus::art {

// Palette indices produced by composeWordmark(). The UI maps them to theme tokens.
enum WordmarkPx : uint8_t {
    kWmClear = 0,
    kWmOutline,     // token: outline
    kWmChromeHi,    // token: chrome_hi   (upper half of the face)
    kWmChromeLo,    // token: chrome_lo   (lower half of the face)
    kWmHighlight,   // token: sparkle     (top-edge shine)
    kWmShadowNear,  // token: pink
    kWmShadowFar,   // token: primary
    kWmGlint,       // token: sparkle     (animated diagonal shine)
    kWmCount
};

struct WordmarkOptions {
    // Diagonal glint position: face pixels with x + y in [glint, glint + 1] become kWmGlint.
    // -1 disables the glint. Sweep it from 0 to wordmarkGlintEnd() to animate.
    int glint = -1;
};

IndexedBitmap composeWordmark(const WordmarkOptions& options = {});

// One past the last glint position that still touches the face.
int wordmarkGlintEnd();

}  // namespace tonus::art

#include "art/wordmark.hpp"

namespace tonus::art {
namespace {

// Each glyph is 16 rows. Rows 2..13 are the cap height; rows 0-1 and 14-15 hold the spikes.
constexpr int kGlyphRows = 16;
constexpr int kChromeSplitRow = 8;  // glyph rows below this are chrome_hi, the rest chrome_lo
constexpr int kLetterGap = 4;

constexpr const char* kT[kGlyphRows] = {
    "#...............#",
    "##.............##",
    "#################",
    "#################",
    "..#############..",
    "......#####......",
    "......#####......",
    "......#####......",
    "......#####......",
    "......#####......",
    "......#####......",
    "......#####......",
    "......#####......",
    "......#####......",
    ".......###.......",
    "........#........",
};

// The O stays plain: spikes on it read as a "Q".
constexpr const char* kO[kGlyphRows] = {
    ".............",
    ".............",
    "..#########..",
    ".###########.",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    ".###########.",
    "..#########..",
    ".............",
    ".............",
};

constexpr const char* kN[kGlyphRows] = {
    ".............#",
    "............##",
    "####......####",
    "#####.....####",
    "######....####",
    "####.##...####",
    "####..##..####",
    "####...##.####",
    "####....######",
    "####.....#####",
    "####......####",
    "####......####",
    "####......####",
    "####......####",
    "##............",
    "#.............",
};

constexpr const char* kU[kGlyphRows] = {
    "#...........#",
    "##.........##",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "####.....####",
    "#############",  // flat bottom, no spike: a pointed U reads as a "V"
    ".###########.",
    ".............",
    ".............",
};

constexpr const char* kS[kGlyphRows] = {
    "............#",
    "...........##",
    ".############",
    "#############",
    "####.........",
    "####.........",
    "####.........",
    "#############",
    "#############",
    ".........####",
    ".........####",
    ".........####",
    "#############",
    "############.",
    "##...........",
    "#............",
};

constexpr const char* const* kLetters[] = {kT, kO, kN, kU, kS};

// The whole word as one mask, built once from the letters.
struct WordMask {
    IndexedBitmap bits;  // 1 = filled
    WordMask() {
        int w = 0;
        for (auto* rows : kLetters) w += AsciiMask{rows, kGlyphRows}.width();
        w += kLetterGap * (static_cast<int>(sizeof(kLetters) / sizeof(kLetters[0])) - 1);
        bits = IndexedBitmap(w, kGlyphRows);
        int x0 = 0;
        for (auto* rows : kLetters) {
            const AsciiMask m{rows, kGlyphRows};
            for (int y = 0; y < kGlyphRows; ++y)
                for (int x = 0; x < m.width(); ++x)
                    if (m.filled(x, y)) bits.set(x0 + x, y, 1);
            x0 += m.width() + kLetterGap;
        }
    }
};

const WordMask& wordMask() {
    static const WordMask mask;
    return mask;
}

// The face sits at (1,1). Behind it is a solid extrusion: the mask repeated 1..kDepth art-px
// down-right, the nearer half in shadow-near colour and the farther half in shadow-far colour.
// One outline goes around the whole silhouette and another around the face.
constexpr int kFaceOffset = 1;
constexpr int kDepth = 4;
constexpr int kPadding = 2 + kDepth;  // outline on both sides + extrusion

void dilateInto(IndexedBitmap& dst, const IndexedBitmap& src, uint8_t value) {
    for (int y = 0; y < dst.height; ++y)
        for (int x = 0; x < dst.width; ++x) {
            bool near = false;
            for (int dy = -1; dy <= 1 && !near; ++dy)
                for (int dx = -1; dx <= 1 && !near; ++dx) near = src.at(x + dx, y + dy) != 0;
            if (near) dst.set(x, y, value);
        }
}

}  // namespace

IndexedBitmap composeWordmark(const WordmarkOptions& options) {
    const IndexedBitmap& mask = wordMask().bits;
    IndexedBitmap out(mask.width + kPadding, mask.height + kPadding);

    // Extrusion, painted far to near so the nearer colour wins where they overlap.
    IndexedBitmap extrusion(out.width, out.height);
    IndexedBitmap face(out.width, out.height);
    for (int k = kDepth; k >= 1; --k) {
        const uint8_t v = k > kDepth / 2 ? kWmShadowFar : kWmShadowNear;
        for (int y = 0; y < mask.height; ++y)
            for (int x = 0; x < mask.width; ++x)
                if (mask.at(x, y)) extrusion.set(kFaceOffset + x + k, kFaceOffset + y + k, v);
    }
    for (int y = 0; y < mask.height; ++y)
        for (int x = 0; x < mask.width; ++x)
            if (mask.at(x, y)) face.set(kFaceOffset + x, kFaceOffset + y, 1);

    IndexedBitmap silhouette = extrusion;
    for (size_t i = 0; i < face.px.size(); ++i)
        if (face.px[i]) silhouette.px[i] = 1;

    dilateInto(out, silhouette, kWmOutline);
    for (size_t i = 0; i < extrusion.px.size(); ++i)
        if (extrusion.px[i]) out.px[i] = extrusion.px[i];
    dilateInto(out, face, kWmOutline);

    for (int y = 0; y < mask.height; ++y) {
        for (int x = 0; x < mask.width; ++x) {
            if (!mask.at(x, y)) continue;
            uint8_t v = y < kChromeSplitRow ? kWmChromeHi : kWmChromeLo;
            if (y < kChromeSplitRow && !mask.at(x, y - 1)) v = kWmHighlight;
            if (options.glint >= 0) {
                const int d = x + y;
                if (d == options.glint || d == options.glint + 1) v = kWmGlint;
            }
            out.set(kFaceOffset + x, kFaceOffset + y, v);
        }
    }
    return out;
}

int wordmarkGlintEnd() {
    const IndexedBitmap& mask = wordMask().bits;
    return mask.width + mask.height;
}

}  // namespace tonus::art

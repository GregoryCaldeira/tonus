#include "art/sprites.hpp"

namespace tonus::art {
namespace {

constexpr const char* kBolt[] = {
    "...####",
    "..####.",
    "..###..",
    ".###...",
    ".######",
    "######.",
    "...###.",
    "..###..",
    "..##...",
    ".##....",
    ".#.....",
    "#......",
};

constexpr const char* kSparkle[] = {
    "..#..",
    "..#..",
    "#####",
    "..#..",
    "..#..",
};

}  // namespace

IndexedBitmap lightningBolt() {
    return composeOutlinedSprite({kBolt, static_cast<int>(sizeof(kBolt) / sizeof(kBolt[0]))}, kSpOutline, kSpFill);
}

IndexedBitmap sparkle() {
    return composeFlatSprite({kSparkle, static_cast<int>(sizeof(kSparkle) / sizeof(kSparkle[0]))}, kSpFill);
}

}  // namespace tonus::art

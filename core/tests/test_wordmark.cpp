#include <catch2/catch_test_macros.hpp>

#include "art/sprites.hpp"
#include "art/wordmark.hpp"

using namespace tonus::art;

namespace {

bool isFace(uint8_t v) {
    return v == kWmChromeHi || v == kWmChromeLo || v == kWmHighlight || v == kWmGlint;
}

WordmarkOptions glintAt(int pos) {
    WordmarkOptions o;
    o.glint = pos;
    return o;
}

int count(const IndexedBitmap& b, uint8_t v) {
    int n = 0;
    for (uint8_t p : b.px) n += p == v;
    return n;
}

}  // namespace

TEST_CASE("wordmark has the expected size and fits the splash at x10") {
    const IndexedBitmap wm = composeWordmark();
    CHECK(wm.width == 92);
    CHECK(wm.height == 22);
    CHECK(wm.width * 10 <= 1280 - 2 * 64);
    CHECK(wm.height * 10 <= 720 / 3);
}

TEST_CASE("wordmark only uses known palette indices") {
    const IndexedBitmap wm = composeWordmark(glintAt(20));
    for (uint8_t p : wm.px) CHECK(p < kWmCount);
}

TEST_CASE("every face pixel is fully enclosed by non-transparent pixels") {
    const IndexedBitmap wm = composeWordmark();
    for (int y = 0; y < wm.height; ++y)
        for (int x = 0; x < wm.width; ++x) {
            if (!isFace(wm.at(x, y))) continue;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx) {
                    INFO("face pixel " << x << "," << y << " touches transparency");
                    REQUIRE(wm.at(x + dx, y + dy) != kWmClear);
                }
        }
}

TEST_CASE("no orphan pixels") {
    const IndexedBitmap wm = composeWordmark();
    for (int y = 0; y < wm.height; ++y)
        for (int x = 0; x < wm.width; ++x) {
            if (wm.at(x, y) == kWmClear) continue;
            bool neighbour = false;
            for (int dy = -1; dy <= 1; ++dy)
                for (int dx = -1; dx <= 1; ++dx)
                    if ((dx || dy) && wm.at(x + dx, y + dy) != kWmClear) neighbour = true;
            REQUIRE(neighbour);
        }
}

TEST_CASE("both shadow layers stay visible") {
    const IndexedBitmap wm = composeWordmark();
    CHECK(count(wm, kWmShadowNear) > 50);
    CHECK(count(wm, kWmShadowFar) > 50);
    CHECK(count(wm, kWmHighlight) > 10);
}

TEST_CASE("composition is deterministic and glint only touches the face") {
    const IndexedBitmap a = composeWordmark();
    const IndexedBitmap b = composeWordmark();
    CHECK(a.px == b.px);

    const IndexedBitmap g = composeWordmark(glintAt(30));
    CHECK(count(g, kWmGlint) > 0);
    for (size_t i = 0; i < a.px.size(); ++i)
        if (g.px[i] != a.px[i]) CHECK((isFace(a.px[i]) && g.px[i] == kWmGlint));

    CHECK(count(composeWordmark(glintAt(wordmarkGlintEnd())), kWmGlint) == 0);
}

TEST_CASE("sprites are outlined or flat as declared") {
    const IndexedBitmap bolt = lightningBolt();
    CHECK(bolt.width == 9);
    CHECK(bolt.height == 14);
    CHECK(count(bolt, kSpOutline) > 0);
    CHECK(count(bolt, kSpFill) > 0);

    const IndexedBitmap star = sparkle();
    CHECK(star.width == 5);
    CHECK(count(star, kSpOutline) == 0);
    CHECK(count(star, kSpFill) == 9);
}

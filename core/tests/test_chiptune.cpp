#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdlib>

#include "audio/chiptune.hpp"

using namespace tonus::audio;

TEST_CASE("midiToHz") {
    CHECK(midiToHz(69) == Catch::Approx(440.0f));
    CHECK(midiToHz(40) == Catch::Approx(82.41f).epsilon(0.001));  // low E on guitar
}

TEST_CASE("boot jingle length, level and silent tail") {
    constexpr int sr = 48000;
    const auto pcm = renderBootJingle(sr);
    const double seconds = static_cast<double>(pcm.size()) / sr;
    CHECK(seconds > 1.0);
    CHECK(seconds < 1.6);

    int peak = 0;
    for (int16_t s : pcm) peak = std::max(peak, std::abs(static_cast<int>(s)));
    CHECK(peak > 32767 * 0.5);
    CHECK(peak <= 32767 * 0.61);  // normalised to 0.6 full scale, never clipping

    const size_t tail = sr / 100;  // last 10 ms
    for (size_t i = pcm.size() - tail; i < pcm.size(); ++i) REQUIRE(pcm[i] == 0);
}

TEST_CASE("notes start without a click") {
    const auto pcm = renderNotes({{0.0f, 0.1f, 440.0f, Wave::Pulse50, 1.0f, 1.0f}}, 48000);
    CHECK(std::abs(static_cast<int>(pcm[0])) < 200);
}

TEST_CASE("SineOsc fills interleaved channels identically") {
    SineOsc osc;
    osc.set(1000.0f, 48000);
    int16_t buf[96 * 2];
    osc.render(buf, 96, 2, 0.5f);
    int peak = 0;
    for (int i = 0; i < 96; ++i) {
        REQUIRE(buf[2 * i] == buf[2 * i + 1]);
        peak = std::max(peak, std::abs(static_cast<int>(buf[2 * i])));
    }
    CHECK(peak > 16000);
    CHECK(peak <= 16384);
}

#include <catch2/catch_test_macros.hpp>

#include <cstring>
#include <string>

#include "i18n/i18n.hpp"

using namespace tonus::i18n;

TEST_CASE("every language has a non-empty string for every key") {
    for (int l = 0; l < static_cast<int>(Lang::Count); ++l)
        for (int s = 0; s < static_cast<int>(Str::Count); ++s) {
            INFO("lang " << kLangCodes[l] << " str " << s);
            REQUIRE(kTable[l][s] != nullptr);
            REQUIRE(std::strlen(kTable[l][s]) > 0);
        }
}

TEST_CASE("switching language changes lookups") {
    setLang(Lang::en);
    CHECK(std::string(tr(Str::splash_tap)) == "TAP TO START");
    setLang(Lang::pt_PT);
    CHECK(std::string(tr(Str::splash_tap)) == "TOCA PARA COMEÇAR");
    setLang(Lang::en);
}

TEST_CASE("out-of-range ids return an empty string, never null") {
    CHECK(std::string(tr(static_cast<Str>(-1))).empty());
    CHECK(std::string(tr(Str::Count)).empty());
}

TEST_CASE("format replaces every placeholder occurrence") {
    setLang(Lang::en);
    CHECK(format(Str::diag_touch, {{"x", "10"}, {"y", "20"}}) == "Touch: 10, 20");
    CHECK(format(Str::diag_heap, {{"internal", "300"}, {"psram", "31000"}}) ==
          "Free RAM: 300 KB internal · 31000 KB PSRAM");
}

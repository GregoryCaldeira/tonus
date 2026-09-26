#include <catch2/catch_test_macros.hpp>

#include "app/boot.hpp"

using namespace tonus;

TEST_CASE("boot runs every step in order and reports the first failure") {
    std::vector<int> order;
    app::BootStatus status;
    const std::vector<app::BootStep> steps = {
        {i18n::Str::boot_stage, [&] { order.push_back(1); return true; }},
        {i18n::Str::boot_amps, [&] { order.push_back(2); return false; }},
        {i18n::Str::boot_mic, [&] { order.push_back(3); return false; }},
        {i18n::Str::boot_ready, nullptr},
    };
    app::runBoot(steps, status);

    CHECK(order == std::vector<int>{1, 2, 3});
    CHECK(status.total == 4);
    CHECK(status.completed == 4);
    CHECK(status.finished);
    CHECK(status.failedLabel == static_cast<int>(i18n::Str::boot_amps));
    CHECK(status.label == static_cast<int>(i18n::Str::boot_ready));
}

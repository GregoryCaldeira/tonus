#pragma once
// Boot sequence runner. Steps run in order on the caller's thread (a boot task on the device,
// a std::thread in the simulator). Progress is published through atomics, so the UI can poll it
// from the LVGL thread without locks.

#include <atomic>
#include <functional>
#include <vector>

#include "i18n/i18n.hpp"

namespace tonus::app {

struct BootStep {
    i18n::Str label;             // shown on the splash while the step runs
    std::function<bool()> run;   // false = failed; boot continues, the failure is reported
};

struct BootStatus {
    std::atomic<int> completed{0};
    std::atomic<int> total{0};
    std::atomic<int> label{-1};        // i18n::Str of the running step, -1 = none
    std::atomic<int> failedLabel{-1};  // i18n::Str of the first failed step, -1 = none
    std::atomic<bool> finished{false};
};

void runBoot(const std::vector<BootStep>& steps, BootStatus& status);

}  // namespace tonus::app

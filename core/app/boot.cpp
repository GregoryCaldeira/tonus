#include "app/boot.hpp"

namespace tonus::app {

void runBoot(const std::vector<BootStep>& steps, BootStatus& status) {
    status.total = static_cast<int>(steps.size());
    status.completed = 0;
    status.finished = false;
    for (const BootStep& step : steps) {
        status.label = static_cast<int>(step.label);
        const bool ok = step.run ? step.run() : true;
        if (!ok && status.failedLabel < 0) status.failedLabel = static_cast<int>(step.label);
        ++status.completed;
    }
    status.label = static_cast<int>(i18n::Str::boot_ready);
    status.finished = true;
}

}  // namespace tonus::app

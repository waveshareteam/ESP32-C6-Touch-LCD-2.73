#pragma once

#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstdint>

namespace badge { namespace detail {
// Check after each bounded flash operation / CPU chunk. taskYIELD() cannot
// schedule IDLE_CPU0 below a runnable priority-2 worker; a blocking delay can.
// The maximum uninterrupted interval is this budget plus one operation/chunk.
class FlashSchedule {
public:
    static constexpr int64_t BUDGET_US = 20000;

    void checkpoint()
    {
        if (esp_timer_get_time() - last_yield_us_ < BUDGET_US) return;
        vTaskDelay(1);
        last_yield_us_ = esp_timer_get_time();
    }

private:
    int64_t last_yield_us_ = esp_timer_get_time();
};
} }

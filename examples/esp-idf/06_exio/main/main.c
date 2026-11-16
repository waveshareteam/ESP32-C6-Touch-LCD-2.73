#include "bsp/esp-bsp.h"
#include "io_extension.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    ESP_ERROR_CHECK(bsp_io_expander_init());
    bool high = false;
    while (true) {
        // These are exposed header outputs; preserve reset and charging inputs.
        ESP_ERROR_CHECK(bsp_test_pin_set(true, IO_EXTENSION_IO_6, high));
        ESP_ERROR_CHECK(bsp_test_pin_set(true, IO_EXTENSION_IO_7, high));
        ESP_ERROR_CHECK(IO_EXTENSION_Write_PWM(high ? 97 : 0));
        ESP_LOGI("exio", "EXIO6/7=%u; backlight=%u%%", high, high ? 97 : 0);
        high = !high;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

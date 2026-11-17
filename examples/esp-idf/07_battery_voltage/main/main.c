#include "demo_ui.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    ESP_ERROR_CHECK(demo_start("BATTERY VOLTAGE", "CH32V006 / 12-bit ADC") ? ESP_OK : ESP_FAIL);
    demo_lock();
    lv_obj_t *voltage = demo_metric("BATTERY TERMINAL VOLTAGE", 14, 84, 292, 106, &lv_font_montserrat_28);
    lv_obj_t *raw = demo_metric("ADC / 4095", 14, 204, 140, 68, &lv_font_montserrat_20);
    lv_obj_t *average = demo_metric("AVERAGING", 166, 204, 140, 68, &lv_font_montserrat_20);
    lv_label_set_text(average, "8 samples");
    demo_unlock();
    while (true) {
        float v = 0;
        uint16_t adc = 0;
        esp_err_t ret = bsp_battery_get_voltage(&v, &adc);
        demo_lock();
        if (ret == ESP_OK) {
            lv_label_set_text_fmt(voltage, "%.3f V", (double)v);
            lv_label_set_text_fmt(raw, "%u", adc);
            demo_status_set("Voltage test / compare with a meter", false);
            ESP_LOGI("battery", "ADC=%u; voltage=%.3f V", adc,v);
        } else {
            lv_label_set_text(voltage, "-- V");
            lv_label_set_text(raw, "--");
            demo_status_set(esp_err_to_name(ret), true);
        }
        demo_unlock();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

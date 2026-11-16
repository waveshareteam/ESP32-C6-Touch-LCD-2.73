#include "demo_ui.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    ESP_ERROR_CHECK(demo_start("ENVIRONMENT", "SHTC3 / temperature & humidity") ? ESP_OK : ESP_FAIL);
    demo_lock();
    lv_obj_t *temperature = demo_metric("TEMPERATURE", 14, 92, 140, 140, &lv_font_montserrat_28);
    lv_obj_t *humidity = demo_metric("HUMIDITY", 166, 92, 140, 140, &lv_font_montserrat_28);
    demo_text(lv_obj_get_parent(temperature), "degrees C", 10, 99, 120, LV_FONT_DEFAULT, 0x63758c);
    demo_text(lv_obj_get_parent(humidity), "% RH", 10, 99, 120, LV_FONT_DEFAULT, 0x63758c);
    demo_text(demo_screen(), "Live measurement / refreshed each second", 14, 248, 292, LV_FONT_DEFAULT, 0x63758c);
    demo_unlock();
    bool ready = false;
    while (true) {
        esp_err_t ret = ready ? ESP_OK : bsp_environment_init();
        ready = ret == ESP_OK;
        float t = 0, h = 0;
        if (ready) ret = bsp_environment_get_data(&t, &h);
        demo_lock();
        if (ret == ESP_OK) {
            lv_label_set_text_fmt(temperature, "%.1f", (double)t);
            lv_label_set_text_fmt(humidity, "%.1f", (double)h);
            demo_status_set("Sensor online / CRC checked", false);
            ESP_LOGI("shtc3", "%.2f C; %.2f %%RH", t, h);
        } else {
            lv_label_set_text(temperature, "--");
            lv_label_set_text(humidity, "--");
            demo_status_set(esp_err_to_name(ret), true);
        }
        demo_unlock();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

#include "demo_ui.h"
#include "esp_log.h"
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    ESP_ERROR_CHECK(demo_start("MOTION SENSOR", "QMI8658 / live 6-axis readings") ? ESP_OK : ESP_FAIL);
    lv_obj_t *values[6];
    const char *axes[] = {"X", "Y", "Z"};
    demo_lock();
    demo_text(demo_screen(), "ACCELERATION / g", 14, 78, 292, LV_FONT_DEFAULT, 0x63758c);
    demo_text(demo_screen(), "ANGULAR VELOCITY / deg/s", 14, 174, 292, LV_FONT_DEFAULT, 0x63758c);
    for (unsigned i = 0; i < 6; ++i)
        values[i] = demo_metric(axes[i % 3], 14 + (i % 3) * 100, i < 3 ? 100 : 196,
                                92, 70, &lv_font_montserrat_20);
    demo_unlock();
    bool ready = false;
    while (true) {
        esp_err_t ret = ready ? ESP_OK : bsp_imu_init();
        ready = ret == ESP_OK;
        bsp_imu_data_t data = {0};
        if (ready) ret = bsp_imu_get_data(&data);
        demo_lock();
        if (ret == ESP_OK) {
            const float numbers[] = {data.accel_x,data.accel_y,data.accel_z,data.gyro_x,data.gyro_y,data.gyro_z};
            for (unsigned i = 0; i < 6; ++i) {
                const char *format = i < 3 ? "%+.2f" : (fabsf(numbers[i]) < 100 ? "%+.1f" : "%+.0f");
                lv_label_set_text_fmt(values[i], format, (double)numbers[i]);
            }
            demo_status_set("Tilt or rotate the board", false);
            ESP_LOGI("imu", "g: %.2f %.2f %.2f; deg/s: %.2f %.2f %.2f", data.accel_x,data.accel_y,data.accel_z,data.gyro_x,data.gyro_y,data.gyro_z);
        } else {
            for (unsigned i = 0; i < 6; ++i) lv_label_set_text(values[i], "--");
            demo_status_set(esp_err_to_name(ret), true);
        }
        demo_unlock();
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

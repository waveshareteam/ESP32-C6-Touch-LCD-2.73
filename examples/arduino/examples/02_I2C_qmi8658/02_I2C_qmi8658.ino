#include <Demo_UI.h>
#include <SensorQMI8658.hpp>
#include <math.h>

static SensorQMI8658 imu;
static lv_obj_t *values[6];
static bool screen_ready, sensor_ready;
static uint32_t last_sample;

static bool start_sensor()
{
    if (!imu.begin(Wire, 0x6B) && !imu.begin(Wire, 0x6A)) return false;
    return imu.configAccelerometer(SensorQMI8658::ACC_RANGE_8G, SensorQMI8658::ACC_ODR_500Hz) == 0 &&
           imu.configGyroscope(SensorQMI8658::GYR_RANGE_512DPS, SensorQMI8658::GYR_ODR_448_4Hz) == 0 &&
           imu.enableAccelerometer() && imu.enableGyroscope();
}
void setup()
{
    Serial.begin(115200);
    delay(200);
    screen_ready = demo_start("MOTION SENSOR", "QMI8658 / live 6-axis readings");
    if (!screen_ready) return;
    const char *axes[] = {"X", "Y", "Z"};
    demo_text(demo_screen(), "ACCELERATION / g", 14, 78, 292, LV_FONT_DEFAULT, 0x63758c);
    demo_text(demo_screen(), "ANGULAR VELOCITY / deg/s", 14, 174, 292, LV_FONT_DEFAULT, 0x63758c);
    for (unsigned i = 0; i < 6; ++i)
        values[i] = demo_metric(axes[i % 3], 14 + (i % 3) * 100, i < 3 ? 100 : 196,
                                92, 70, &lv_font_montserrat_20);
    last_sample = millis() - 200;
}
void loop()
{
    demo_tick();
    if (screen_ready && millis() - last_sample >= 200) {
        last_sample = millis();
        if (!sensor_ready) sensor_ready = start_sensor();
        float numbers[6] = {};
        bool ok = sensor_ready && imu.getAccelerometer(numbers[0],numbers[1],numbers[2]) &&
                  imu.getGyroscope(numbers[3],numbers[4],numbers[5]);
        if (ok) {
            for (unsigned i = 0; i < 6; ++i) {
                const char *format = i < 3 ? "%+.2f" : (fabsf(numbers[i]) < 100 ? "%+.1f" : "%+.0f");
                char text[24];
                snprintf(text, sizeof(text), format, (double)numbers[i]);
                lv_label_set_text(values[i], text);
            }
            demo_status_set("Tilt or rotate the board", false);
            Serial.printf("g: %.2f %.2f %.2f; deg/s: %.2f %.2f %.2f\n",
                          numbers[0],numbers[1],numbers[2],numbers[3],numbers[4],numbers[5]);
        } else {
            for (auto *value : values) lv_label_set_text(value, "--");
            demo_status_set("QMI8658 unavailable / retrying", true);
        }
    }
    delay(5);
}

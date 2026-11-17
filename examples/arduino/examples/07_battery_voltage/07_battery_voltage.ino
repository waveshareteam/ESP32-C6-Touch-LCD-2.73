#include <Demo_UI.h>

static lv_obj_t *voltage_label, *raw_label;
static bool screen_ready;
static uint32_t last_sample;
void setup()
{
    Serial.begin(115200);
    delay(200);
    screen_ready = demo_start("BATTERY VOLTAGE", "CH32V006 / 12-bit ADC");
    if (!screen_ready) return;
    voltage_label = demo_metric("BATTERY TERMINAL VOLTAGE", 14, 84, 292, 106, &lv_font_montserrat_28);
    raw_label = demo_metric("ADC / 4095", 14, 204, 140, 68, &lv_font_montserrat_20);
    lv_obj_t *average = demo_metric("AVERAGING", 166, 204, 140, 68, &lv_font_montserrat_20);
    lv_label_set_text(average, "8 samples");
    last_sample = millis() - 1000;
}
void loop()
{
    demo_tick();
    if (screen_ready && millis() - last_sample >= 1000) {
        last_sample = millis();
        float voltage = 0;
        uint16_t raw = 0;
        if (Board_ReadBatteryVoltage(voltage, raw)) {
            char text[24];
            snprintf(text, sizeof(text), "%.3f V", voltage);
            lv_label_set_text(voltage_label, text);
            lv_label_set_text_fmt(raw_label, "%u", raw);
            demo_status_set("Voltage test / compare with a meter", false);
            Serial.printf("ADC=%u; voltage=%.3f V\n",raw,voltage);
        } else {
            lv_label_set_text(voltage_label, "-- V");
            lv_label_set_text(raw_label, "--");
            demo_status_set("Battery ADC read failed / retrying", true);
        }
    }
    delay(5);
}

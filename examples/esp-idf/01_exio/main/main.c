#include "demo_ui.h"
#include <stdio.h>
typedef struct {
    unsigned pin;
    bool known, high;
    lv_obj_t *button, *label;
} output_t;
static output_t outputs[11];

static bool set_output(output_t *output, bool high)
{
#ifdef ARDUINO
    const bool ok = Board_IO_SetOutput(output->pin, high);
#else
    const bool ok = bsp_test_pin_set(true, output->pin, high) == ESP_OK;
#endif
    if (ok) {
        output->known = true;
        output->high = high;
        lv_label_set_text_fmt(output->label, "EXIO%u\n%s", output->pin, high ? "HIGH" : "LOW");
        lv_obj_set_style_bg_color(output->button, lv_color_hex(high ? 0x2178cf : 0xdce5f0), 0);
        lv_obj_set_style_text_color(output->button, high ? lv_color_white() : lv_color_hex(0x17365c), 0);
    }
    return ok;
}
static void output_clicked(lv_event_t *event)
{
    output_t *output = (output_t *)lv_event_get_user_data(event);
    if (set_output(output, !output->known || !output->high)) {
        char message[64];
        snprintf(message, sizeof(message), "EXIO%u output set %s", output->pin, output->high ? "HIGH" : "LOW");
        demo_status_set(message, false);
    } else demo_status_set("IO write failed; last state retained", true);
}
static void all_low_clicked(lv_event_t *event)
{
    (void)event;
    bool ok = true;
    for (unsigned i = 0; i < 11; ++i) {
        if (!set_output(&outputs[i], false)) ok = false;
    }
    demo_status_set(ok ? "All 11 outputs set LOW" : "Some IO writes failed", !ok);
}
static void create_outputs(void)
{
    demo_text(demo_screen(), "0: LCD RST   1: Touch RST\n2: Backlight   3: Charge input", 14, 72, 292, LV_FONT_DEFAULT, 0x63758c);
    bool ok = true;
    for (unsigned i = 0; i < 11; ++i) {
        output_t *output = &outputs[i];
        output->pin = i + 4;
        output->button = demo_button(14 + (i % 4) * 75, 112 + (i / 4) * 48, 67, 42);
        output->label = lv_label_create(output->button);
        lv_label_set_text_fmt(output->label, "EXIO%u\n--", output->pin);
        lv_obj_set_style_text_align(output->label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(output->label);
        lv_obj_add_event_cb(output->button, output_clicked, LV_EVENT_CLICKED, output);
        if (!set_output(output, false)) ok = false;
    }
    lv_obj_t *button = demo_button(239, 208, 67, 42);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x17365c), 0);
    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, "ALL\nLOW");
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(label);
    lv_obj_add_event_cb(button, all_low_clicked, LV_EVENT_CLICKED, NULL);
    demo_text(demo_screen(), "Tap a pin to toggle its output", 14, 259, 292, LV_FONT_DEFAULT, 0x63758c);
    demo_status_set(ok ? "EXIO4-14 ready; backlight stays on" : "IO initialization incomplete", !ok);
}

void app_main(void)
{
    ESP_ERROR_CHECK(demo_start("IO EXPANDER", "CH32V006 / 11 header outputs") ? ESP_OK : ESP_FAIL);
    demo_lock();
    create_outputs();
    demo_unlock();
}

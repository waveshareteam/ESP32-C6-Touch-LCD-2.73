#include "pinout_init.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"
#include "io_extension.h"

#define PINOUT_PIN_COUNT 19

typedef enum {
    PINOUT_GPIO,
    PINOUT_EXIO,
} pinout_type_t;

typedef struct {
    pinout_type_t type;
    uint8_t pin;
    lv_obj_t *row;
    lv_obj_t *state_label;
    lv_obj_t *dot;
    bool writable;
    bool initialized;
    bool high;
} pinout_item_t;

static const char *TAG = "app_pinout";
static pinout_item_t s_pins[PINOUT_PIN_COUNT];
static size_t s_pin_count;
static lv_obj_t *s_total_label;

static void render_pin(const pinout_item_t *item)
{
    if (!item->initialized) {
        lv_label_set_text(item->state_label, "--");
        lv_obj_set_style_text_color(item->state_label, lv_color_hex(0x82909B), LV_PART_MAIN);
        lv_obj_set_style_bg_color(item->dot, lv_color_hex(0x586873), LV_PART_MAIN);
        return;
    }

    if (item->high) {
        lv_label_set_text(item->state_label, "H");
        lv_obj_set_style_text_color(item->state_label, lv_color_hex(0x3DE6A5), LV_PART_MAIN);
        lv_obj_set_style_bg_color(item->dot, lv_color_hex(0x46D798), LV_PART_MAIN);
    } else {
        lv_label_set_text(item->state_label, "L");
        lv_obj_set_style_text_color(item->state_label, lv_color_hex(0x8AA0B2), LV_PART_MAIN);
        lv_obj_set_style_bg_color(item->dot, lv_color_hex(0x586873), LV_PART_MAIN);
    }
}

static void update_high_count(void)
{
    unsigned int high_count = 0;

    for (size_t i = 0; i < s_pin_count; ++i) {
        if (s_pins[i].initialized && s_pins[i].high) {
            ++high_count;
        }
    }

    if (s_total_label != NULL) {
        lv_label_set_text_fmt(s_total_label, "%u / %u H", high_count, (unsigned int)s_pin_count);
    }
}

static esp_err_t write_pin(const pinout_item_t *item, bool high)
{
    return bsp_test_pin_set(item->type == PINOUT_EXIO, item->pin, high);
}

static void pin_clicked_cb(lv_event_t *event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    pinout_item_t *item = lv_event_get_user_data(event);
    if (item == NULL || !item->writable) {
        return;
    }

    const bool new_level = item->initialized ? !item->high : true;
    esp_err_t err = write_pin(item, new_level);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set pin %u: %s", item->pin, esp_err_to_name(err));
        return;
    }

    item->initialized = true;
    item->high = new_level;
    render_pin(item);
    update_high_count();
}

static void add_pin(pinout_type_t type, uint8_t pin, bool writable, lv_obj_t *row,
                    lv_obj_t *state_label, lv_obj_t *dot)
{
    if (s_pin_count >= PINOUT_PIN_COUNT) {
        return;
    }

    pinout_item_t *item = &s_pins[s_pin_count++];
    *item = (pinout_item_t) {
        .type = type,
        .pin = pin,
        .row = row,
        .state_label = state_label,
        .dot = dot,
        .writable = writable,
        .initialized = false,
        .high = false,
    };

    if (writable) {
        lv_obj_add_flag(row, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_event_cb(row, pin_clicked_cb, LV_EVENT_CLICKED, item);
    } else {
        lv_obj_remove_flag(row, LV_OBJ_FLAG_CLICKABLE);
    }
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    render_pin(item);
}

static void create_gpio_row(lv_obj_t *parent, int32_t y, const char *name,
                            lv_obj_t **row_out, lv_obj_t **state_out,
                            lv_obj_t **dot_out)
{
    lv_obj_t *row = lv_obj_create(parent);
    lv_obj_set_pos(row, 0, y);
    lv_obj_set_size(row, 147, 21);
    lv_obj_set_scrollbar_mode(row, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(row, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(row, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(row, lv_color_hex(0x2A3741), LV_PART_MAIN);
    lv_obj_set_style_border_side(row, LV_BORDER_SIDE_BOTTOM, LV_PART_MAIN);
    lv_obj_set_style_radius(row, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(row, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(row, 0, LV_PART_MAIN);

    lv_obj_t *name_label = lv_label_create(row);
    lv_obj_set_pos(name_label, 5, 3);
    lv_obj_set_size(name_label, 49, 15);
    lv_label_set_text(name_label, name);
    lv_obj_set_style_text_color(name_label, lv_color_hex(0xE8EEF3), LV_PART_MAIN);
    lv_obj_set_style_text_font(name_label, &lv_font_montserratMedium_12, LV_PART_MAIN);
    lv_obj_set_style_text_align(name_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);

    lv_obj_t *state = lv_label_create(row);
    lv_obj_set_pos(state, 110, 2);
    lv_obj_set_size(state, 12, 12);
    lv_obj_set_style_text_align(state, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
    lv_obj_set_style_text_font(state, &lv_font_montserratMedium_12, LV_PART_MAIN);

    lv_obj_t *dot = lv_obj_create(row);
    lv_obj_set_pos(dot, 130, 4);
    lv_obj_set_size(dot, 10, 10);
    lv_obj_set_scrollbar_mode(dot, LV_SCROLLBAR_MODE_OFF);
    lv_obj_remove_flag(dot, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(dot, 5, LV_PART_MAIN);
    lv_obj_set_style_border_width(dot, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(dot, lv_color_hex(0x2195F6), LV_PART_MAIN);
    lv_obj_set_style_border_opa(dot, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_pad_all(dot, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(dot, 0, LV_PART_MAIN);

    *row_out = row;
    *state_out = state;
    *dot_out = dot;
}

void pinout_ui_init(lv_ui *ui)
{
    s_pin_count = 0;
    s_total_label = ui->screen_total;

    lv_obj_set_scrollbar_mode(ui->screen_EXIO_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scroll_dir(ui->screen_EXIO_cont, LV_DIR_NONE);
    lv_obj_remove_flag(ui->screen_EXIO_cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_label_set_text(ui->screen_label_gpio28_name, "GPIO9");
    lv_label_set_text(ui->screen_label_gpio25_name, "GPIO3");
    lv_label_set_text(ui->screen_label_gpio24_name, "GPIO15");
    lv_label_set_text(ui->screen_label_gpio23_name, "GPIO23");

    add_pin(PINOUT_GPIO, GPIO_NUM_9, true, ui->screen_row_GPIO28,
            ui->screen_label_gpio28_state, ui->screen_dot_gpio28);
    add_pin(PINOUT_GPIO, GPIO_NUM_3, true, ui->screen_row_GPIO25,
            ui->screen_label_gpio25_state, ui->screen_dot_gpio25);
    add_pin(PINOUT_GPIO, GPIO_NUM_15, true, ui->screen_row_GPIO24,
            ui->screen_label_gpio24_state, ui->screen_dot_gpio24);
    add_pin(PINOUT_GPIO, GPIO_NUM_23, true, ui->screen_row_GPIO23,
            ui->screen_label_gpio23_state, ui->screen_dot_gpio23);

    lv_obj_t *gpio13_row;
    lv_obj_t *gpio13_state;
    lv_obj_t *gpio13_dot;
    create_gpio_row(ui->screen_GPIO_cont, 112, "GPIO13",
                    &gpio13_row, &gpio13_state, &gpio13_dot);
    add_pin(PINOUT_GPIO, GPIO_NUM_13, true, gpio13_row,
            gpio13_state, gpio13_dot);

    lv_obj_t *gpio12_row;
    lv_obj_t *gpio12_state;
    lv_obj_t *gpio12_dot;
    create_gpio_row(ui->screen_GPIO_cont, 133, "GPIO12",
                    &gpio12_row, &gpio12_state, &gpio12_dot);
    add_pin(PINOUT_GPIO, GPIO_NUM_12, true, gpio12_row,
            gpio12_state, gpio12_dot);

    lv_obj_t *gpio22_row;
    lv_obj_t *gpio22_state;
    lv_obj_t *gpio22_dot;
    create_gpio_row(ui->screen_GPIO_cont, 154, "GPIO22",
                    &gpio22_row, &gpio22_state, &gpio22_dot);
    add_pin(PINOUT_GPIO, GPIO_NUM_22, true, gpio22_row,
            gpio22_state, gpio22_dot);

    lv_obj_t *gpio21_row;
    lv_obj_t *gpio21_state;
    lv_obj_t *gpio21_dot;
    create_gpio_row(ui->screen_GPIO_cont, 175, "GPIO21",
                    &gpio21_row, &gpio21_state, &gpio21_dot);
    add_pin(PINOUT_GPIO, GPIO_NUM_21, true, gpio21_row,
            gpio21_state, gpio21_dot);
    lv_label_set_text(ui->screen_GPIO_Pins_num, "8 pins");

    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_14, true, ui->screen_row_EXIO14,
            ui->screen_label_exio14_state, ui->screen_dot_exio14);
    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_13, true, ui->screen_row_EXIO13,
            ui->screen_label_exio13_state, ui->screen_dot_exio13);
    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_12, true, ui->screen_row_EXIO12,
            ui->screen_label_exio12_state, ui->screen_dot_exio12);
    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_11, true, ui->screen_row_EXIO11,
            ui->screen_label_exio11_state, ui->screen_dot_exio11);
    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_10, true, ui->screen_row_EXIO10,
            ui->screen_label_exio10_state, ui->screen_dot_exio10);
    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_9, true, ui->screen_row_EXIO9,
            ui->screen_label_exio9_state, ui->screen_dot_exio9);
    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_8, true, ui->screen_row_EXIO8,
            ui->screen_label_exio8_state, ui->screen_dot_exio8);
    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_7, true, ui->screen_row_EXIO7,
            ui->screen_label_exio7_state, ui->screen_dot_exio7);
    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_6, true, ui->screen_row_EXIO6,
            ui->screen_label_exio6_state, ui->screen_dot_exio6);
    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_5, true, ui->screen_row_EXIO5,
            ui->screen_label_exio5_state, ui->screen_dot_exio5);
    add_pin(PINOUT_EXIO, IO_EXTENSION_IO_4, true, ui->screen_row_EXIO4,
            ui->screen_label_exio4_state, ui->screen_dot_exio4);

    update_high_count();
}

void pinout_ui_deinit(void)
{
    bsp_test_pins_release();
    s_pin_count = 0;
    s_total_label = NULL;
}

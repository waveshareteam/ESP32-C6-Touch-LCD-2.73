#pragma once

#include <Arduino.h>
#include <lvgl.h>

// Wire must already use SDA=20, SCL=2. Reset is CH32V006 EXIO1.
// As in the IDF BSP, INT is unused and coordinates are polled.
bool gt911_init(bool (*set_reset)(bool high));
lv_indev_t *gt911_register_lvgl_input(lv_disp_t *display);

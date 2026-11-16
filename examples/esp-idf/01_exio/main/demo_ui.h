/* SPDX-License-Identifier: Apache-2.0 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <lvgl.h>
#ifdef ARDUINO
#include <Arduino.h>
#include <Display_ILI9488.h>
#include <gt911_bsp.h>
#else
#include "bsp/esp-bsp.h"
#endif

static lv_obj_t *demo_footer;
#ifdef ARDUINO
static bool demo_ready;
static uint32_t demo_last_tick;
static lv_color_t demo_pixels[LCD_WIDTH * LCD_DRAW_BUF_HEIGHT];
static lv_disp_draw_buf_t demo_buffer;
static lv_disp_drv_t demo_display;
static void demo_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *pixels)
{
    LCD_addWindow(area->x1, area->y1, area->x2, area->y2, (uint16_t *)pixels);
    lv_disp_flush_ready(drv);
}
#endif

static inline lv_obj_t *demo_screen(void)
{
#if LVGL_VERSION_MAJOR >= 9
    return lv_screen_active();
#else
    return lv_scr_act();
#endif
}
static inline void demo_no_scroll(lv_obj_t *obj)
{
#if LVGL_VERSION_MAJOR >= 9
    lv_obj_remove_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
#else
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
#endif
}
static inline void demo_lock(void)
{
#ifndef ARDUINO
    ESP_ERROR_CHECK(bsp_display_lock(UINT32_MAX) ? ESP_OK : ESP_ERR_TIMEOUT);
#endif
}
static inline void demo_unlock(void)
{
#ifndef ARDUINO
    bsp_display_unlock();
#endif
}
static inline lv_obj_t *demo_text(lv_obj_t *parent, const char *text, int x, int y,
                                  int width, const lv_font_t *font, uint32_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, width);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_label_set_text(label, text);
    return label;
}
static inline lv_obj_t *demo_card(int x, int y, int width, int height)
{
    lv_obj_t *card = lv_obj_create(demo_screen());
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, width, height);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_set_style_bg_color(card, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    demo_no_scroll(card);
    return card;
}
static inline lv_obj_t *demo_metric(const char *title, int x, int y, int width, int height,
                                    const lv_font_t *font)
{
    lv_obj_t *card = demo_card(x, y, width, height);
    demo_text(card, title, 10, 8, width - 20, LV_FONT_DEFAULT, 0x63758c);
    return demo_text(card, "--", 10, 32, width - 20, font, 0x17365c);
}
static inline void demo_status_set(const char *text, bool error)
{
    lv_obj_set_style_text_color(demo_footer, lv_color_hex(error ? 0xc34040 : 0x63758c), 0);
    lv_label_set_text(demo_footer, text);
}
static inline lv_obj_t *demo_button(int x, int y, int width, int height)
{
#if LVGL_VERSION_MAJOR >= 9
    lv_obj_t *button = lv_button_create(demo_screen());
#else
    lv_obj_t *button = lv_btn_create(demo_screen());
#endif
    lv_obj_set_pos(button, x, y);
    lv_obj_set_size(button, width, height);
    lv_obj_set_style_pad_all(button, 0, 0);
    lv_obj_set_style_radius(button, 9, 0);
    lv_obj_set_style_shadow_width(button, 0, 0);
    demo_no_scroll(button);
    return button;
}
static inline bool demo_start(const char *title, const char *subtitle)
{
#ifdef ARDUINO
    if (!Board_IO_Init() || !LCD_Init()) {
        Serial.println("Display initialization failed");
        return false;
    }
    const bool touch = gt911_init(Board_Touch_SetReset);
    lv_init();
    lv_disp_draw_buf_init(&demo_buffer, demo_pixels, nullptr, LCD_WIDTH * LCD_DRAW_BUF_HEIGHT);
    lv_disp_drv_init(&demo_display);
    demo_display.hor_res = LCD_WIDTH;
    demo_display.ver_res = LCD_HEIGHT;
    demo_display.flush_cb = demo_flush;
    demo_display.draw_buf = &demo_buffer;
    lv_disp_t *display = lv_disp_drv_register(&demo_display);
    if (!display) return false;
    if (touch) gt911_register_lvgl_input(display);
    demo_ready = true;
    demo_last_tick = millis();
#else
    if (!bsp_display_start()) return false;
#endif
    demo_lock();
    lv_obj_t *screen = demo_screen();
    demo_no_scroll(screen);
    lv_obj_set_style_pad_all(screen, 0, 0);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xf2f5fa), 0);
    lv_obj_set_style_text_color(screen, lv_color_hex(0x17365c), 0);
    demo_text(screen, title, 14, 13, 292, &lv_font_montserrat_20, 0x17365c);
    demo_text(screen, subtitle, 14, 43, 292, LV_FONT_DEFAULT, 0x63758c);
    demo_footer = demo_text(screen, "Initializing...", 14, 282, 292, LV_FONT_DEFAULT, 0x63758c);
    demo_unlock();
#ifdef ARDUINO
    lv_refr_now(nullptr);
    if (!Board_SetBacklight(97)) Serial.println("Backlight write failed");
    return true;
#else
    return bsp_display_show() == ESP_OK;
#endif
}
#ifdef ARDUINO
static inline void demo_tick(void)
{
    if (!demo_ready) return;
    const uint32_t now = millis();
    lv_tick_inc(now - demo_last_tick);
    demo_last_tick = now;
    lv_timer_handler();
}
#endif

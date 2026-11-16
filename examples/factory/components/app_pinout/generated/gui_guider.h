/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"


typedef struct
{
  
	lv_obj_t *screen;
	bool screen_del;
	lv_obj_t *screen_EXIO_cont;
	lv_obj_t *screen_row_EXIO14;
	lv_obj_t *screen_dot_exio14;
	lv_obj_t *screen_label_exio14_state;
	lv_obj_t *screen_label_exio14_name;
	lv_obj_t *screen_row_EXIO13;
	lv_obj_t *screen_dot_exio13;
	lv_obj_t *screen_label_exio13_state;
	lv_obj_t *screen_label_exio13_name;
	lv_obj_t *screen_row_EXIO12;
	lv_obj_t *screen_dot_exio12;
	lv_obj_t *screen_label_exio12_state;
	lv_obj_t *screen_label_exio12_name;
	lv_obj_t *screen_row_EXIO11;
	lv_obj_t *screen_dot_exio11;
	lv_obj_t *screen_label_exio11_state;
	lv_obj_t *screen_label_exio11_name;
	lv_obj_t *screen_row_EXIO10;
	lv_obj_t *screen_dot_exio10;
	lv_obj_t *screen_label_exio10_state;
	lv_obj_t *screen_label_exio10_name;
	lv_obj_t *screen_row_EXIO9;
	lv_obj_t *screen_dot_exio9;
	lv_obj_t *screen_label_exio9_state;
	lv_obj_t *screen_label_exio9_name;
	lv_obj_t *screen_row_EXIO8;
	lv_obj_t *screen_dot_exio8;
	lv_obj_t *screen_label_exio8_state;
	lv_obj_t *screen_label_exio8_name;
	lv_obj_t *screen_row_EXIO7;
	lv_obj_t *screen_dot_exio7;
	lv_obj_t *screen_label_exio7_state;
	lv_obj_t *screen_label_exio7_name;
	lv_obj_t *screen_row_EXIO6;
	lv_obj_t *screen_dot_exio6;
	lv_obj_t *screen_label_exio6_state;
	lv_obj_t *screen_label_exio6_name;
	lv_obj_t *screen_row_EXIO5;
	lv_obj_t *screen_dot_exio5;
	lv_obj_t *screen_label_exio5_state;
	lv_obj_t *screen_label_exio5_name;
	lv_obj_t *screen_row_EXIO4;
	lv_obj_t *screen_dot_exio4;
	lv_obj_t *screen_label_exio4_state;
	lv_obj_t *screen_label_exio4_name;
	lv_obj_t *screen_EXIO_Pins_num;
	lv_obj_t *screen_EXIO_title;
	lv_obj_t *screen_GPIO_cont;
	lv_obj_t *screen_row_GPIO28;
	lv_obj_t *screen_dot_gpio28;
	lv_obj_t *screen_label_gpio28_name;
	lv_obj_t *screen_label_gpio28_state;
	lv_obj_t *screen_row_GPIO25;
	lv_obj_t *screen_dot_gpio25;
	lv_obj_t *screen_label_gpio25_state;
	lv_obj_t *screen_label_gpio25_name;
	lv_obj_t *screen_row_GPIO24;
	lv_obj_t *screen_dot_gpio24;
	lv_obj_t *screen_label_gpio24_state;
	lv_obj_t *screen_label_gpio24_name;
	lv_obj_t *screen_row_GPIO23;
	lv_obj_t *screen_dot_gpio23;
	lv_obj_t *screen_label_gpio23_state;
	lv_obj_t *screen_label_gpio23_name;
	lv_obj_t *screen_GPIO_Pins_num;
	lv_obj_t *screen_GPIO_title;
	lv_obj_t *screen_total;
	lv_obj_t *screen_title;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_bottom_layer(void);

void setup_ui(lv_ui *ui);

void video_play(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_screen(lv_ui *ui);

LV_FONT_DECLARE(lv_font_montserratMedium_12)
LV_FONT_DECLARE(lv_font_montserratMedium_11)
LV_FONT_DECLARE(lv_font_montserratMedium_13)
LV_FONT_DECLARE(lv_font_montserratMedium_18)


#ifdef __cplusplus
}
#endif
#endif

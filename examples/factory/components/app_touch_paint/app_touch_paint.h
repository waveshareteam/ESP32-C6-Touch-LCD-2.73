#pragma once
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
bool app_touch_paint_open(lv_obj_t *parent, void (*back)(void *), void *context);
void app_touch_paint_close(void);
void app_touch_paint_pause(void);
void app_touch_paint_resume(void);
#ifdef __cplusplus
}
#endif

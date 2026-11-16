#pragma once
#include "lvgl.h"
#ifdef __cplusplus
extern "C" {
#endif
bool app_pinout_open(lv_obj_t *parent);
void app_pinout_close(void);
#ifdef __cplusplus
}
#endif

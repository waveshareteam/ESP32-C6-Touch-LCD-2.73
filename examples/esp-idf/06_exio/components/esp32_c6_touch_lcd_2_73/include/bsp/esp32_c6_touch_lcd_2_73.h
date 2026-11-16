/* SPDX-License-Identifier: Apache-2.0 */
/* ESP32-C6-Touch-LCD-2.73: shared bus, panel and touch interfaces. */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "sdkconfig.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lv_adapter.h"
#include "bsp/config.h"
#include "bsp/display.h"
#define BSP_BOARD_ESP32_C6_TOUCH_LCD_2_73 1
#define BSP_I2C_NUM CONFIG_BSP_I2C_NUM
#define BSP_I2C_FREQ_HZ CONFIG_BSP_I2C_CLK_SPEED_HZ
#define BSP_I2C_SCL GPIO_NUM_2
#define BSP_I2C_SDA GPIO_NUM_20
#define BSP_LCD_DATA0 GPIO_NUM_1
#define BSP_SPI_MISO GPIO_NUM_18
#define BSP_LCD_PCLK GPIO_NUM_0
#define BSP_LCD_DC GPIO_NUM_19
#define BSP_LCD_CS GPIO_NUM_8
#define BSP_LCD_RST GPIO_NUM_NC
#define BSP_LCD_BACKLIGHT GPIO_NUM_NC
#define BSP_LCD_TOUCH_RST GPIO_NUM_NC
#define BSP_LCD_TOUCH_INT GPIO_NUM_NC
#define BSP_LCD_PIXEL_CLOCK_HZ (60 * 1000 * 1000)
#define BSP_LCD_SPI_NUM SPI2_HOST
#define BSP_LCD_SPI_HOST BSP_LCD_SPI_NUM
#define BSP_SD_SPI_MOSI BSP_LCD_DATA0
#define BSP_SD_SPI_MISO BSP_SPI_MISO
#define BSP_SD_SPI_CLK BSP_LCD_PCLK
#define BSP_SD_SPI_CS GPIO_NUM_10
#define BSP_SD_SDCS BSP_SD_SPI_CS
#define BSP_SD_MOUNT_POINT CONFIG_BSP_SD_MOUNT_POINT
#define BSP_BUTTON_BOOT GPIO_NUM_9
#define BSP_BUTTON_LEFT GPIO_NUM_4
#define BSP_BUTTON_RIGHT GPIO_NUM_5
#define BSP_BUTTON_BOOT_ACTIVE_LEVEL 0
#define BSP_CAPS_DISPLAY 1
#define BSP_CAPS_TOUCH 1
#define BSP_CAPS_BUTTONS 1
#define BSP_CAPS_AUDIO 0
#define BSP_CAPS_AUDIO_SPEAKER 0
#define BSP_CAPS_AUDIO_MIC 0
#define BSP_CAPS_SDCARD 1
#define BSP_CAPS_IMU 1
#define BSP_CAPS_CAMERA 0
#ifdef __cplusplus
extern "C" {
#endif
esp_err_t bsp_i2c_init(void);
esp_err_t bsp_i2c_deinit(void);
i2c_master_bus_handle_t bsp_i2c_get_handle(void);
typedef struct {
    esp_lv_adapter_config_t lv_adapter_cfg;
    esp_lv_adapter_rotation_t rotation;
    esp_lv_adapter_tear_avoid_mode_t tear_avoid_mode;
    struct { unsigned swap_xy, mirror_x, mirror_y; } touch_flags;
} bsp_display_cfg_t;
lv_display_t *bsp_display_start(void);
lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *config);
lv_display_t *bsp_display_get_disp_dev(void);
lv_indev_t *bsp_display_get_input_dev(void);
/* Timeout is in ms; UINT32_MAX waits indefinitely. */
bool bsp_display_lock(uint32_t timeout_ms);
void bsp_display_unlock(void);
esp_err_t bsp_display_show(void);
int bsp_display_brightness_get(void);
#ifdef __cplusplus
}
#endif
#include "bsp/touch.h"
#include "bsp/board.h"

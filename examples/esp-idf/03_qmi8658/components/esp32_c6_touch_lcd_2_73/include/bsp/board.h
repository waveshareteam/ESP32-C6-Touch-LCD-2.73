/* SPDX-License-Identifier: Apache-2.0 */
/* Board startup, buttons, sensors, storage and Wi-Fi hardware interfaces. */
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"
#include "lvgl.h"
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Startup results: ESP_OK means ready; this is not a live health check. */
typedef struct {
    esp_err_t i2c, expander, power, display, touch;
    esp_err_t button, audio, rtc, imu, environment, sdcard, wifi;
} bsp_init_status_t;

/* Initialize buses, backlight-off, NVS, SD, display/touch, then other peripherals.
 * Optional failures are recorded in status. Create the UI before calling show(). */
esp_err_t bsp_init(void);
/* Read-only BSP-owned startup results. */
const bsp_init_status_t *bsp_get_init_status(void);
/* Initialize only I2C and CH32V003; pin configuration belongs to each peripheral. */
esp_err_t bsp_io_expander_init(void);
/* Average eight 12-bit ADC samples; nominal 3.3 V reference and 3:1 divider. */
esp_err_t bsp_battery_get_voltage(float *voltage, uint16_t *raw);
/* Borrowed handles; NULL before initialization. */

lv_display_t *bsp_display_get(void);
esp_lcd_touch_handle_t bsp_touch_get_handle(void);

/* Active-low BOOT, LEFT and RIGHT buttons. */
typedef enum { BSP_BUTTON_ID_BOOT, BSP_BUTTON_ID_LEFT, BSP_BUTTON_ID_RIGHT } bsp_button_id_t;
/* Start 20 ms sampling; count presses after three stable samples. */
esp_err_t bsp_user_button_init(void);
/* Read raw state into pressed; no debounce in this getter. */
esp_err_t bsp_button_get_pressed(bsp_button_id_t button, bool *pressed);
typedef struct {
    bool pressed;
    uint32_t press_count;
} bsp_button_state_t;
/* Atomic debounced state/count, using the same sampler for all three keys.
 * Count once on press after a stable release; holding does not auto-repeat. */
esp_err_t bsp_button_get_state(bsp_button_id_t button, bsp_button_state_t *state);
/* BOOT's global debounced press count; compare snapshots for per-APP counts. */
uint32_t bsp_user_button_get_press_count(void);

/* RTC stores a Beijing calendar. The network worker seeds system time from it
 * at startup and writes SNTP corrections back; GUI reads use wifi_manager_get_clock(). */
typedef struct {
    uint16_t year; /* 1970..2069 */
    uint8_t month, day, hour, minute, second, weekday; /* Month 1..12; weekday 0=Sunday */
} bsp_sensor_datetime_t;
typedef struct {
    float accel_x, accel_y, accel_z; /* g */
    float gyro_x, gyro_y, gyro_z; /* deg/s */
    float temperature_c; /* IMU die temperature, Celsius */
} bsp_imu_data_t;

/* Each sensor initializes its I2C prerequisite only. Invalid RTC dates use a fallback. */
esp_err_t bsp_rtc_init(void);
esp_err_t bsp_rtc_get_datetime(bsp_sensor_datetime_t *datetime);
esp_err_t bsp_rtc_set_datetime(const bsp_sensor_datetime_t *datetime);
/* QMI8658 acceleration, angular velocity and die temperature. */
esp_err_t bsp_imu_init(void);
esp_err_t bsp_imu_get_data(bsp_imu_data_t *data);
/* SHTC3 ambient temperature/humidity; reads block for measurement (~15 ms).
 * Use a worker task. Outputs are Celsius and %RH, updated only on success. */
esp_err_t bsp_environment_init(void);
esp_err_t bsp_environment_get_data(float *temperature_c, float *humidity_percent);

typedef struct { bool mounted;
    uint64_t capacity_bytes, total_bytes, free_bytes; /* Card capacity, FAT total/free space */
} bsp_sdcard_info_t;
/* Mount FAT at /sdcard over shared SPI; never auto-format. */
esp_err_t bsp_sdcard_mount(void);
/* Acquire before opening files; prevents unmount, not concurrent file access. */
esp_err_t bsp_sdcard_acquire(void);
/* Close files before release; release does not unmount. */
void bsp_sdcard_release(void);
/* Reject active leases; never free the SPI host shared with LCD. */
esp_err_t bsp_sdcard_unmount(void);
/* Query mounted card/FAT capacity; may block. Does not mount the card. */
esp_err_t bsp_sdcard_get_info(bsp_sdcard_info_t *info);

/* Initialize driver/netifs after NVS; credentials and connection belong to the service. */
esp_err_t bsp_wifi_init(void);
/* Start the initialized driver with 2.4 GHz operation. */
esp_err_t bsp_wifi_start(void);
/* Charge detect: EXIO3 high means charging; preserve output on failure. */
esp_err_t bsp_battery_get_charging(bool *charging);
/* PinsTest access is limited to the board's exposed test pins. */
esp_err_t bsp_test_pin_set(bool extended, unsigned pin, bool high);
void bsp_test_pins_release(void);
#ifdef __cplusplus
}
#endif

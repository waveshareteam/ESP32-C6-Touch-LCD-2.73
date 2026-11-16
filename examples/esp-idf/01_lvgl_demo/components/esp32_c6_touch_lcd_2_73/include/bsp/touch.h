#pragma once
#include "esp_lcd_touch.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief BSP touch configuration structure
 *
 */
typedef struct {
    void *dummy;    /*!< Prepared for future use. */
} bsp_touch_config_t;

/**
 * @brief Create the GT911 touchscreen
 *
 * @param[in]  cfg       Display and touch orientation configuration
 * @param[out] ret_touch Touchscreen handle
 * @return ESP_OK on success, otherwise an ESP-IDF error code
 */
esp_err_t bsp_touch_new(const bsp_display_cfg_t *cfg, esp_lcd_touch_handle_t *ret_touch);


#ifdef __cplusplus
}
#endif

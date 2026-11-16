#include <string.h>
#include <stdlib.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "dirent.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <time.h>
#include <sys/time.h>

#include "bsp/esp32_c6_touch_lcd_2_73.h"
#include "bsp/touch.h"
#include "bsp_err_check.h"
#include "esp_lcd_ili9488.h"
#include "esp_lcd_touch_gt911.h"
#include "bsp_board_internal.h"
#include "io_extension.h"

static const char *TAG = "esp32_c6_touch_lcd_2_73";


static i2c_master_bus_handle_t i2c_handle = NULL;
static bool i2c_initialized = false;
static bool spi_initialized = false;
static esp_lcd_touch_handle_t tp;   // LCD touch handle
static esp_lcd_panel_handle_t panel_handle = NULL; // LCD panel handle
static esp_lcd_panel_io_handle_t io_handle = NULL;
#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
static lv_indev_t *disp_indev = NULL;
static lv_display_t *disp_drv = NULL;
#endif // (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
static uint8_t brightness = 100;
static bool backlight_released;


/**************************************************************************************************
 * I2C init
 **************************************************************************************************/
esp_err_t bsp_i2c_init(void)
{
    /* I2C was initialized before */
    if (i2c_initialized) {
        return ESP_OK;
    }

    const i2c_master_bus_config_t i2c_config = {
        .i2c_port = BSP_I2C_NUM,
        .sda_io_num = BSP_I2C_SDA,
        .scl_io_num = BSP_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    BSP_ERROR_CHECK_RETURN_ERR(i2c_new_master_bus(&i2c_config, &i2c_handle));

    i2c_initialized = true;
    return ESP_OK;
}

esp_err_t bsp_i2c_deinit(void)
{
    if (!i2c_initialized) return ESP_OK;
    if (bsp_board_i2c_in_use() || tp) return ESP_ERR_INVALID_STATE;
    BSP_ERROR_CHECK_RETURN_ERR(i2c_del_master_bus(i2c_handle));
    i2c_handle = NULL;
    i2c_initialized = false;
    return ESP_OK;
}

i2c_master_bus_handle_t bsp_i2c_get_handle(void)
{
    return i2c_handle;
}



esp_err_t bsp_spi_bus_init(void)
{
    /* SPI was initialized before */
    if (spi_initialized) {
        return ESP_OK;
    }

    ESP_LOGD(TAG, "Initialize SPI bus");
    const spi_bus_config_t buscfg = {
        .sclk_io_num = BSP_LCD_PCLK,
        .mosi_io_num = BSP_LCD_DATA0,
        .miso_io_num = BSP_SPI_MISO,
        .quadwp_io_num = GPIO_NUM_NC,
        .quadhd_io_num = GPIO_NUM_NC,
        .max_transfer_sz = BSP_LCD_H_RES * BSP_LCD_DRAW_BUFFER_HEIGHT * 3,
    };
    ESP_RETURN_ON_ERROR(spi_bus_initialize(BSP_LCD_SPI_NUM, &buscfg, SPI_DMA_CH_AUTO), TAG, "SPI init failed");
    spi_initialized = true;
    return ESP_OK;
}



esp_err_t bsp_display_brightness_init(void)
{
    ESP_RETURN_ON_ERROR(bsp_io_expander_init(), TAG, "Expander");
    return IO_EXTENSION_Write_PWM(0);
}

esp_err_t bsp_display_brightness_set(int brightness_percent)
{
    if (brightness_percent > 100) {
        brightness_percent = 100;
    } else if (brightness_percent < 0) {
        brightness_percent = 0;
    }

    if (backlight_released) {
        ESP_RETURN_ON_ERROR(IO_EXTENSION_Write_PWM(brightness_percent), TAG, "Backlight");
    }
    brightness = (uint8_t)brightness_percent;
    ESP_LOGI(TAG, "Backlight: %d%%", brightness_percent);
    return ESP_OK;
}

int bsp_display_brightness_get(void)
{
    return (int)brightness;
}

esp_err_t bsp_display_backlight_off(void)
{
    ESP_LOGI(TAG, "Backlight off");
    return bsp_display_brightness_set(0);
}

esp_err_t bsp_display_backlight_on(void)
{
    ESP_LOGI(TAG, "Backlight on");
    return bsp_display_brightness_set(100);
}

/**************************************************************************************************
 * LCD init
 **************************************************************************************************/

esp_err_t bsp_display_new(const bsp_display_config_t *config, esp_lcd_panel_handle_t *ret_panel, esp_lcd_panel_io_handle_t *ret_io)
{
    BSP_ERROR_CHECK_RETURN_ERR(bsp_io_expander_init());

    IO_EXTENSION_Output(IO_EXTENSION_IO_0, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    IO_EXTENSION_Output(IO_EXTENSION_IO_0, 1);

    esp_err_t ret = ESP_OK;

    ESP_LOGD(TAG, "Initialize SPI bus");
    ESP_RETURN_ON_ERROR(bsp_spi_bus_init(), TAG, "LCD SPI");

    ESP_LOGD(TAG, "Install panel IO");
    const esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = BSP_LCD_DC,
        .cs_gpio_num = BSP_LCD_CS,
        .pclk_hz = BSP_LCD_PIXEL_CLOCK_HZ,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = CONFIG_BSP_LCD_TRANS_QUEUE_DEPTH,
    };
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_NUM, &io_config, ret_io), err, TAG, "New panel IO failed");

    ESP_LOGD(TAG, "Install LCD driver");
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = BSP_LCD_RST, // LCD reset is controlled by IO expander
        .rgb_ele_order = BSP_LCD_COLOR_SPACE,
        .bits_per_pixel = BSP_LCD_BITS_PER_PIXEL,
        .data_endian = LCD_RGB_DATA_ENDIAN_BIG,
        
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9488(
                        *ret_io,
                        &panel_config,
                        BSP_LCD_H_RES * BSP_LCD_DRAW_BUFFER_HEIGHT,
                        ret_panel));

    esp_lcd_panel_reset(*ret_panel);
    esp_lcd_panel_init(*ret_panel);
    esp_lcd_panel_invert_color(*ret_panel, true);
    esp_lcd_panel_disp_on_off(*ret_panel, true);
    esp_lcd_panel_swap_xy(*ret_panel, false);
    esp_lcd_panel_mirror(*ret_panel,false,false);

    // Pre-fill the controller's own GRAM with black before LVGL starts. The
    // launcher is transferred in multiple chunks; untouched areas will now
    // remain black instead of showing the panel's power-on white contents.
    const size_t clear_pixels = BSP_LCD_H_RES * BSP_LCD_DRAW_BUFFER_HEIGHT;
    uint16_t *black_line_buffer = calloc(clear_pixels, sizeof(uint16_t));
    if (black_line_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate LCD black clear buffer");
        return ESP_ERR_NO_MEM;
    }

    for (int y = 0; y < BSP_LCD_V_RES; y += BSP_LCD_DRAW_BUFFER_HEIGHT) {
        const int y_end = MIN(y + BSP_LCD_DRAW_BUFFER_HEIGHT, BSP_LCD_V_RES);
        ret = esp_lcd_panel_draw_bitmap(*ret_panel, 0, y, BSP_LCD_H_RES, y_end, black_line_buffer);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to clear LCD GRAM at y=%d: %s", y, esp_err_to_name(ret));
            free(black_line_buffer);
            return ret;
        }
        // draw_bitmap() uses queued SPI transfers. Keep the single conversion
        // buffer stable until this chunk has left the bus before reusing it.
        vTaskDelay(pdMS_TO_TICKS(5));
    }
    free(black_line_buffer);
    ESP_LOGI(TAG, "LCD GRAM cleared to black");


    return ret;

err:
    if (*ret_panel) {
        esp_lcd_panel_del(*ret_panel);
    }
    if (*ret_io) {
        esp_lcd_panel_io_del(*ret_io);
    }
    /* SPI is shared with the SD card; do not free it here. */
    return ret;
}


/**************************************************************************************************
 *
 * Touch init (GT911)
 *
 **************************************************************************************************/
esp_err_t bsp_touch_new(const bsp_display_cfg_t *cfg, esp_lcd_touch_handle_t *ret_touch)
{
    assert(cfg != NULL);
    assert(ret_touch != NULL);

    BSP_ERROR_CHECK_RETURN_ERR(bsp_i2c_init());

    /* GT911 reset is connected to IO1 of the CH32V003 IO expander. */
    IO_EXTENSION_Output(IO_EXTENSION_IO_1, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    IO_EXTENSION_Output(IO_EXTENSION_IO_1, 1);
    vTaskDelay(pdMS_TO_TICKS(50));

    uint8_t found_addr = 0;
    const uint8_t gt911_addresses[] = {
        ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,
        ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP,
    };

    for (size_t i = 0; i < sizeof(gt911_addresses) / sizeof(gt911_addresses[0]); i++) {
        const uint8_t address = gt911_addresses[i];
        ESP_LOGI(TAG, "Probe GT911 at I2C address 0x%02X", address);
        if (i2c_master_probe(i2c_handle, address, 100) == ESP_OK) {
            found_addr = address;
            break;
        }
    }

    if (found_addr == 0) {
        ESP_LOGE(TAG, "GT911 not found at I2C address 0x%02X or 0x%02X",
                 ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS,
                 ESP_LCD_TOUCH_IO_I2C_GT911_ADDRESS_BACKUP);
        return ESP_ERR_NOT_FOUND;
    }
    ESP_LOGI(TAG, "GT911 found at I2C address 0x%02X", found_addr);

    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = BSP_LCD_H_RES,
        .y_max = BSP_LCD_V_RES,
        .rst_gpio_num = BSP_LCD_TOUCH_RST,
        .int_gpio_num = BSP_LCD_TOUCH_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = cfg->touch_flags.swap_xy,
            .mirror_x = cfg->touch_flags.mirror_x,
            .mirror_y = cfg->touch_flags.mirror_y,
        },
    };

    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_GT911_CONFIG();
    tp_io_config.dev_addr = found_addr;
    tp_io_config.scl_speed_hz = CONFIG_BSP_I2C_CLK_SPEED_HZ;

    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_err_t ret = esp_lcd_new_panel_io_i2c(i2c_handle, &tp_io_config, &tp_io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GT911 panel IO init failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_lcd_touch_new_i2c_gt911(tp_io_handle, &tp_cfg, ret_touch);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GT911 touch init failed: %s", esp_err_to_name(ret));
        esp_lcd_panel_io_del(tp_io_handle);
        return ret;
    }

    tp = *ret_touch;
    ESP_LOGI(TAG, "GT911 touch init success");
    return ESP_OK;
}

/**************************************************************************************************
 * LVGL init
 **************************************************************************************************/

#if (BSP_CONFIG_NO_GRAPHIC_LIB == 0)
static lv_display_t *bsp_display_lcd_init(const bsp_display_cfg_t *cfg)
{
    assert(cfg != NULL);

    bsp_display_config_t disp_config = {
        .max_transfer_sz = BSP_LCD_H_RES * BSP_LCD_DRAW_BUFFER_HEIGHT * 3,
    };
    BSP_ERROR_CHECK_RETURN_NULL(bsp_display_new(&disp_config, &panel_handle, &io_handle));

    ESP_LOGD(TAG, "Add LCD screen");
    esp_lv_adapter_display_config_t disp_cfg = {
        .panel = panel_handle,
        .panel_io = io_handle,
        .profile = {
            .interface = ESP_LV_ADAPTER_PANEL_IF_OTHER,
            .rotation = cfg->rotation,
            .hor_res = BSP_LCD_H_RES,
            .ver_res = BSP_LCD_V_RES,
            .buffer_height = BSP_LCD_DRAW_BUFFER_HEIGHT,
            .use_psram = false,
            .enable_ppa_accel = false,
            .require_double_buffer = false,
        },
        .tear_avoid_mode = cfg->tear_avoid_mode,
    };

    lv_display_t *disp = esp_lv_adapter_register_display(&disp_cfg);
    if (!disp)
    {
        return NULL;
    }

    return disp;
}


static lv_indev_t *bsp_display_indev_init(const bsp_display_cfg_t *cfg, lv_display_t *disp)
{
    assert(cfg != NULL);
    BSP_ERROR_CHECK_RETURN_NULL(bsp_touch_new(cfg, &tp));
    const esp_lv_adapter_touch_config_t touch_cfg = ESP_LV_ADAPTER_TOUCH_DEFAULT_CONFIG(disp, tp);

    return esp_lv_adapter_register_touch(&touch_cfg);
}

lv_display_t *bsp_display_start(void)
{
    bsp_display_cfg_t cfg = {
        .lv_adapter_cfg = ESP_LV_ADAPTER_DEFAULT_CONFIG(),
        .rotation = ESP_LV_ADAPTER_ROTATE_0,
        .tear_avoid_mode = ESP_LV_ADAPTER_TEAR_AVOID_MODE_NONE,
        .touch_flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 0}};
    return bsp_display_start_with_config(&cfg);
}

lv_display_t *bsp_display_start_with_config(const bsp_display_cfg_t *cfg)
{
    lv_display_t *disp;
    if (disp_drv) return disp_drv;

    assert(cfg != NULL);
    BSP_ERROR_CHECK_RETURN_NULL(esp_lv_adapter_init(&cfg->lv_adapter_cfg));

    BSP_NULL_CHECK(disp = bsp_display_lcd_init(cfg), NULL);

    BSP_NULL_CHECK(disp_indev = bsp_display_indev_init(cfg, disp), NULL);
    
    ESP_ERROR_CHECK(esp_lv_adapter_start());

    disp_drv = disp;
    return disp_drv;
}

lv_display_t *bsp_display_get_disp_dev(void)
{
    return disp_drv;
}

lv_indev_t *bsp_display_get_input_dev(void)
{
    return disp_indev;
}

bool bsp_display_lock(uint32_t timeout_ms)
{
    return esp_lv_adapter_lock(timeout_ms) == ESP_OK;
}

void bsp_display_unlock(void)
{
    esp_lv_adapter_unlock();
}

#endif // (BSP_CONFIG_NO_GRAPHIC_LIB == 0)

lv_display_t *bsp_display_get(void) { return disp_drv; }
esp_lcd_touch_handle_t bsp_touch_get_handle(void) { return tp; }
esp_err_t bsp_display_show(void)
{
    if (!disp_drv || !io_handle) return ESP_ERR_INVALID_STATE;
    if (!bsp_display_lock(UINT32_MAX)) return ESP_ERR_TIMEOUT;
    lv_obj_invalidate(lv_display_get_screen_active(disp_drv));
    lv_refr_now(disp_drv);
    esp_err_t ret = esp_lcd_panel_io_tx_param(io_handle, -1, NULL, 0);
    if (ret == ESP_OK) {
        ret = IO_EXTENSION_Write_PWM(brightness);
        backlight_released = ret == ESP_OK;
    }
    bsp_display_unlock();
    return ret;
}

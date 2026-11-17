/* SPDX-License-Identifier: Apache-2.0 */
#include "Display_ILI9488.h"
#include "esp_lcd_ili9488.h"
#include "driver/spi_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "freertos/semphr.h"

static esp_lcd_panel_io_handle_t io;
static esp_lcd_panel_handle_t panel;
static SemaphoreHandle_t transfer_done;

static bool color_done(esp_lcd_panel_io_handle_t, esp_lcd_panel_io_event_data_t *, void *)
{
    BaseType_t wake = pdFALSE;
    xSemaphoreGiveFromISR(transfer_done, &wake);
    return wake == pdTRUE;
}

bool LCD_Init()
{
    if (panel) return true;
    if (!Board_IO_Init() || !Board_SetBacklight(0) || !Board_LCD_Reset()) return false;
    pinMode(BOARD_SD_CS_PIN, OUTPUT);
    digitalWrite(BOARD_SD_CS_PIN, HIGH);
    transfer_done = xSemaphoreCreateBinary();
    if (!transfer_done) return false;
    spi_bus_config_t bus = {};
    bus.mosi_io_num = BOARD_LCD_MOSI_PIN;
    bus.miso_io_num = BOARD_LCD_MISO_PIN;
    bus.sclk_io_num = BOARD_LCD_SCLK_PIN;
    bus.quadwp_io_num = bus.quadhd_io_num = -1;
    bus.max_transfer_sz = LCD_WIDTH * LCD_DRAW_BUF_HEIGHT * 3;
    if (spi_bus_initialize(SPI2_HOST, &bus, SPI_DMA_CH_AUTO) != ESP_OK) return false;
    esp_lcd_panel_io_spi_config_t config = {};
    config.cs_gpio_num = BOARD_LCD_CS_PIN;
    config.dc_gpio_num = BOARD_LCD_DC_PIN;
    config.pclk_hz = 60 * 1000 * 1000;
    config.trans_queue_depth = 2;
    config.lcd_cmd_bits = config.lcd_param_bits = 8;
    config.on_color_trans_done = color_done;
    if (esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI2_HOST, &config, &io) != ESP_OK) return false;
    esp_lcd_panel_dev_config_t device = {};
    device.reset_gpio_num = -1;
    device.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_BGR;
    device.bits_per_pixel = 18;
    device.data_endian = LCD_RGB_DATA_ENDIAN_BIG;
    if (esp_lcd_new_panel_ili9488(io, &device, LCD_WIDTH * LCD_DRAW_BUF_HEIGHT, &panel) != ESP_OK) return false;
    return esp_lcd_panel_reset(panel) == ESP_OK &&
           esp_lcd_panel_init(panel) == ESP_OK &&
           esp_lcd_panel_invert_color(panel, true) == ESP_OK &&
           esp_lcd_panel_swap_xy(panel, false) == ESP_OK &&
           esp_lcd_panel_mirror(panel, false, false) == ESP_OK &&
           esp_lcd_panel_disp_on_off(panel, true) == ESP_OK;
}

void LCD_addWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t *pixels)
{
    ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, x1, y1, x2 + 1, y2 + 1, pixels));
    // ILI9488 reuses a single RGB666 conversion buffer. Wait before reusing it
    // or letting LVGL render into its RGB565 draw buffer again.
    if (xSemaphoreTake(transfer_done, pdMS_TO_TICKS(1000)) != pdTRUE) {
        ESP_ERROR_CHECK(ESP_ERR_TIMEOUT);
    }
}

void Set_Backlight(uint8_t percent) { Board_SetBacklight(percent); }
void Backlight_Init() { Set_Backlight(97); }

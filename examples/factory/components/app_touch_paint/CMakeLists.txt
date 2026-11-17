idf_component_register(SRCS "app_touch_paint.c" "assets/image_draw.c"
    INCLUDE_DIRS "." REQUIRES lvgl esp_mm esp32_c6_touch_lcd_2_73 esp_lvgl_adapter)

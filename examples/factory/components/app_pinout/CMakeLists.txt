set(PROJ_SRC ${CMAKE_CURRENT_LIST_DIR})

file(GLOB_RECURSE PROJ_SRCS_C "${PROJ_SRC}/*.c")

idf_component_register(
    SRCS ${PROJ_SRCS_C}
    INCLUDE_DIRS
        "${PROJ_SRC}"
        "${PROJ_SRC}/generated"
        "${PROJ_SRC}/generated/guider_fonts"
        "${PROJ_SRC}/generated/guider_customer_fonts"
        "${PROJ_SRC}/generated/images"
        "${PROJ_SRC}/custom"
        "${PROJ_SRC}/pinout_init"
    REQUIRES
        lvgl__lvgl
        io_extension
        esp32_c6_touch_lcd_2_73
        esp_driver_gpio
)

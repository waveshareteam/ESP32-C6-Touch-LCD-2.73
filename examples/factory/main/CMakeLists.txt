idf_component_register(SRCS "main.cpp" "dark/scr_bg.c"
    INCLUDE_DIRS "."
    REQUIRES esp32_c6_touch_lcd_2_73 brookesia_core Settings Weather Sensor
             Games Qmi8658Ball ButtonTest DisplayTest LegacyApps network_service Watch Badge)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)

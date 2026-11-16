idf_component_register(SRCS "Badge.cpp" "badge_service.cpp" "badge_flash_schedule.cpp" "assets/img_app_badge.c"
    INCLUDE_DIRS "." REQUIRES app_support network_service
    PRIV_REQUIRES spiffs esp_partition esp_http_server json esp_timer
    EMBED_FILES "badge.html")
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)

# IDF 5.5's port callbacks are private. Compile against the installed headers;
# keep the scheduling adaptation here without changing IDF or vendoring SPIFFS.
idf_component_get_property(badge_spiffs_dir spiffs COMPONENT_DIR)
target_include_directories(${COMPONENT_LIB} PRIVATE
    "${badge_spiffs_dir}" "${badge_spiffs_dir}/spiffs/src")
target_link_libraries(${COMPONENT_LIB} INTERFACE
    "-Wl,--wrap=spiffs_api_read"
    "-Wl,--wrap=spiffs_api_write"
    "-Wl,--wrap=spiffs_api_erase"
    # Badge's archive can precede SPIFFS without a linker rescan group. Force
    # extraction of the adapter even when its only callers are wrapped refs.
    "-Wl,--undefined=__wrap_spiffs_api_read"
    "-Wl,--undefined=__wrap_spiffs_api_write"
    "-Wl,--undefined=__wrap_spiffs_api_erase")

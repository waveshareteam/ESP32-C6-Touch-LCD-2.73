idf_component_register(SRCS "Watch.cpp" "assets/img_app_watch.c" INCLUDE_DIRS "."
    REQUIRES app_support network_service)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)

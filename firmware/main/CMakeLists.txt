idf_component_register(
    SRCS "main.cpp" "hal_tab5.cpp"
    INCLUDE_DIRS "."
    REQUIRES core ui
    PRIV_REQUIRES nvs_flash esp_timer)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wall -Wextra)

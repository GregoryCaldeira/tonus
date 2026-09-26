# tonus ui: LVGL 9 screens and widgets shared by the firmware and the simulator.
# Builds as an ESP-IDF component or a plain static library (the caller provides an `lvgl` target).
cmake_minimum_required(VERSION 3.16)

file(GLOB TONUS_FONT_SRCS "${CMAKE_CURRENT_LIST_DIR}/fonts/*.c")
set(TONUS_UI_SRCS
    app.cpp
    theme.cpp
    screens/diagnostics.cpp
    screens/home.cpp
    screens/splash.cpp
    widgets/pixel_art.cpp
    widgets/pixel_box.cpp
    widgets/segment_bar.cpp
    ${TONUS_FONT_SRCS}
)

if(ESP_PLATFORM)
    idf_component_register(
        SRCS ${TONUS_UI_SRCS}
        INCLUDE_DIRS "."
        REQUIRES core lvgl__lvgl)
    target_compile_definitions(${COMPONENT_LIB} PRIVATE LV_LVGL_H_INCLUDE_SIMPLE)
    return()
endif()

add_library(tonus_ui STATIC ${TONUS_UI_SRCS})
target_include_directories(tonus_ui PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(tonus_ui PUBLIC tonus_core lvgl)
target_compile_features(tonus_ui PUBLIC cxx_std_17)
target_compile_definitions(tonus_ui PRIVATE LV_LVGL_H_INCLUDE_SIMPLE)

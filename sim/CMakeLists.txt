# Tonus desktop simulator: the same core/ and ui/ code as the firmware, in a 1280x720 SDL window.
cmake_minimum_required(VERSION 3.16)
project(tonus_sim C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

# LVGL: keep the version in step with firmware/dependencies.lock (lvgl/lvgl).
include(FetchContent)
set(LV_CONF_PATH "${CMAKE_CURRENT_LIST_DIR}/lv_conf.h" CACHE STRING "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG v9.6.0
    GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(lvgl)
target_include_directories(lvgl PUBLIC "${CMAKE_CURRENT_LIST_DIR}")
target_link_libraries(lvgl PUBLIC SDL2::SDL2)

add_subdirectory(../core "${CMAKE_BINARY_DIR}/core")
add_subdirectory(../ui "${CMAKE_BINARY_DIR}/ui")

add_executable(tonus_sim main.cpp hal_sim.cpp)
target_link_libraries(tonus_sim PRIVATE tonus_ui tonus_core lvgl SDL2::SDL2)
target_compile_options(tonus_sim PRIVATE -Wall -Wextra)

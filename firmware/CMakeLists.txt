cmake_minimum_required(VERSION 3.16)

# Shared platform-free code and the LVGL UI live outside the IDF project so the simulator can use them too.
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/../core" "${CMAKE_CURRENT_LIST_DIR}/../ui")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(tonus)

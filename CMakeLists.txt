cmake_minimum_required(VERSION 3.5)

if (NOT DEFINED PROJECT_NAME)
   include($ENV{IDF_PATH}/tools/cmake/project.cmake)
   list(APPEND EXTRA_COMPONENT_DIRS components/lvgl_esp32_drivers components/lvgl_esp32_drivers/lvgl_touch components/lvgl_esp32_drivers/lvgl_tft)
   project(handheld_RnD_IDF)
else()
    message(FATAL_ERROR "LV PORT ESP32: This must be a project's main CMakeLists.txt.")
endif()
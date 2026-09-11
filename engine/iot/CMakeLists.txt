# FILENAME: CMakeLists.txt
# Standalone sub-project for BASIC++ IoT & Microcontroller Edition

cmake_minimum_required(VERSION 3.15)
project(basicpp_iot C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(IOT_SOURCES
    src/esp32_hal.c
    src/esp32_regs.c
    src/iot_sensors.c
    src/iot_net.c
    src/esp32_serial.c
)

add_library(libiot STATIC ${IOT_SOURCES})
target_include_directories(libiot PUBLIC include ../include)

if(MSVC)
    target_compile_options(libiot PRIVATE /W4 /WX /std:c17 /D_CRT_SECURE_NO_WARNINGS)
else()
    target_compile_options(libiot PRIVATE -Wall -Wextra -Wpedantic -Werror -std=c17)
    target_compile_definitions(libiot PRIVATE $<$<BOOL:${UNIX}>:_POSIX_C_SOURCE=200809L _XOPEN_SOURCE=700 _GNU_SOURCE>)
endif()

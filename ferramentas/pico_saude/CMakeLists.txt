cmake_minimum_required(VERSION 3.20)
set(PICO_BOARD pico CACHE STRING "Board type")
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(pico_saude C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()

add_executable(pico_saude saude.c)
target_link_libraries(pico_saude
    pico_stdlib pico_unique_id pico_multicore pico_flash
    hardware_flash hardware_adc hardware_clocks)
pico_enable_stdio_usb(pico_saude 1)
pico_enable_stdio_uart(pico_saude 0)
pico_add_extra_outputs(pico_saude)

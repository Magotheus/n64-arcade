cmake_minimum_required(VERSION 3.20)

# Placa: pico_w (padrão) ou pico (Pico comum / clones). Ver build.ps1 -Placa.
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pico SDK 2.x (a extensão "Raspberry Pi Pico" do VS Code configura isto)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

project(n64_mechanical C CXX ASM)
set(CMAKE_C_STANDARD 11)
pico_sdk_init()

# libjoybus fixada na tag v0.9.0 (baixada como .tar.gz, não precisa de git)
set(JOYBUS_BACKEND rp2xxx)      # nome usado na documentação atual (main)
set(JOYBUS_BACKEND_RP2XXX 1)    # nome usado pela tag v0.9.0
include(FetchContent)
FetchContent_Declare(libjoybus
    URL https://github.com/loopj/libjoybus/archive/refs/tags/v0.9.0.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(libjoybus)

add_executable(n64_mechanical src/main.c)
target_link_libraries(n64_mechanical pico_stdlib joybus)

# Roda o programa inteiro da RAM: a resposta Joybus tem prazo de poucos µs e,
# rodando da flash (XIP), uma falta de cache pode atrasar a interrupção.
pico_set_binary_type(n64_mechanical copy_to_ram)

# Saída de diagnóstico pela USB (monitor serial)
pico_enable_stdio_usb(n64_mechanical 1)
pico_enable_stdio_uart(n64_mechanical 0)

pico_add_extra_outputs(n64_mechanical)

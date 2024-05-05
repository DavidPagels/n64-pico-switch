cmake_minimum_required(VERSION 3.25.1)

include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
include($ENV{PICO_SDK_PATH}/tools/CMakeLists.txt)
project(retro_pico_switch C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)
set(PICO_BOARD pico_w)

project(retro_pico_switch C CXX ASM)

pico_sdk_init()
add_subdirectory(src)

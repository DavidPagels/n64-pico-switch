# Options:
# SWITCH_BLUETOOTH: Remove for USB
#add_compile_definitions(SWITCH_BLUETOOTH)

add_subdirectory(otherController)
add_subdirectory(switchController)

add_executable(${PROJECT_NAME} main.cpp)
target_link_libraries(
    ${PROJECT_NAME} 
    pico_stdlib
    #switchBluetooth
    switchUsb
    xbox_ble
    pico_cyw43_arch_none
)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR}/include)

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
pico_add_extra_outputs(${PROJECT_NAME})
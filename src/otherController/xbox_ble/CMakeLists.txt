add_library(xbox_ble XboxController_BLE.cpp hog_host.c)
target_link_libraries(
    xbox_ble
    controller
    pico_cyw43_arch_none 
    pico_stdlib 
    pico_btstack_cyw43
    pico_btstack_ble
    hardware_pwm
    hardware_gpio
)
target_include_directories(xbox_ble PUBLIC ${PROJECT_SOURCE_DIR}/include)

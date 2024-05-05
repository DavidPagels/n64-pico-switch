add_library(switchUsb SwitchUsb.cpp SwitchCommon.cpp)
target_link_libraries(switchUsb pico_stdlib pico_rand hardware_pio tinyusb_device)
target_include_directories(switchUsb PUBLIC ${PROJECT_SOURCE_DIR}/include)

#add_library(switchBluetooth SwitchBluetooth.cpp SwitchCommon.cpp)
#target_link_libraries(switchBluetooth 
#  pico_stdlib
#  pico_rand
#  pico_btstack_cyw43
#  pico_btstack_hci_transport_cyw43
#  pico_btstack_ble
#  pico_btstack_classic
#  pico_cyw43_arch_threadsafe_background
#  )
#target_compile_definitions(switchBluetooth PUBLIC CYW43_LWIP=0)
#target_include_directories(switchBluetooth PUBLIC ${PROJECT_SOURCE_DIR}/include)

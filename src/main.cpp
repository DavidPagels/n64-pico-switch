#include <stdio.h>
#include <string.h>

#include "SwitchCommon.h"
#include "XboxController_BLE.h"
#include "btstack_run_loop.h"
#include "hardware/gpio.h"
#include "hog_host.h"
#include "pico/stdlib.h"

#ifdef SWITCH_BLUETOOTH

# include "SwitchBluetooth.h"
# include "btstack.h"
SwitchCommon *switchCommon = new SwitchBluetooth();
static btstack_packet_callback_registration_t hci_event_callback_registration;

static void packet_handler_wrapper(
    uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t packet_size
)
{
    packet_handler( (SwitchBluetooth *)switchCommon, packet_type, packet );
}

static void hid_report_data_callback_wrapper(
    uint16_t cid, hid_report_type_t report_type, uint16_t report_id, int report_size,
    uint8_t *report
)
{
    // USB report callback includes 2 bytes excluded here, prepending 2 bytes to
    // keep alignment
    hid_report_data_callback( switchCommon, report_id, report - 1, report_size + 1 );
}

#else /* SWITCH_BLUETOOTH */

# include "SwitchUsb.h"
# include "tusb.h"

SwitchCommon *switchCommon = new SwitchUsb();
void tud_hid_set_report_cb(
    uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer,
    uint16_t bufsize
)
{
    hid_report_data_callback( switchCommon, (uint16_t)buffer[0], (uint8_t *)buffer, bufsize );
}

#endif /* SWITCH_BLUETOOTH */

int main( void )
{
    stdio_init_all();

    // Initialize BTstack UART
    if ( cyw43_arch_init() )
    {
        printf( "Wi-Fi init failed\n" );
        return -1;
    }

    cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 );
    sleep_ms( 500 );
    cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 );
    sleep_ms( 500 );

    printf( "BTstack on Pico starting...\n" );

    InitParams *initParams = new InitParams();
    initParams->pin = 18;
    initParams->controllerType = Xbox;
    Controller *controller = new XboxController_BLE( initParams );

    controller->init();
    switchCommon->init( controller );

#ifdef SWITCH_BLUETOOTH
    hci_event_callback_registration.callback = &packet_handler_wrapper;
    hci_add_event_handler( &hci_event_callback_registration );

    hid_device_register_packet_handler( &packet_handler_wrapper );
    hid_device_register_report_data_callback( &hid_report_data_callback_wrapper );

    // turn on!
    hci_power_control( HCI_POWER_ON );

#endif

    // Enter run loop (forever)
    btstack_run_loop_execute();
}

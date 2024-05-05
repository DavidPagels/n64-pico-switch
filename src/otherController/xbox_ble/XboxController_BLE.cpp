#include "XboxController_BLE.h"

#include "Controller.pio.h"
#include "hog_host.h"

#define DEBUG_BLINK( t )                                 \
    do                                                   \
    {                                                    \
        cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 1 ); \
        sleep_ms( t );                                   \
        cyw43_arch_gpio_put( CYW43_WL_GPIO_LED_PIN, 0 ); \
        sleep_ms( t );                                   \
    } while ( 0 )

// Analog to digital trigger threshold
#define TRIG_THRESHOLD ( 0x30 )

// Convert an Xbox joystick position to a Switch joystick position
#define XB2SW_JOY( pos, old_max, new_max ) ( ( ( pos ) * new_max ) / old_max )

#define SW_JOY_MAX SWITCH_JOYSTICK_MAX

// TODO: Add support for rumble
// TODO: Can support be added for motion controls?

// Event handler function
static void XboxControllerEventHandler( void *obj, const uint8_t *data, uint16_t size )
{
    // Cast the object to a pointer to the XboxControllerState_t struct
    auto state = static_cast<XboxController_BLE *>( obj )->getState();

    // Update the controller state
    state->dpad_state = data[XB_BYTE_DPAD];

    state->btn_A = ( ( XB_MASK_A & data[XB_BYTE_BUTTONS_A] ) ? 1 : 0 );
    state->btn_B = ( ( XB_MASK_B & data[XB_BYTE_BUTTONS_A] ) ? 1 : 0 );
    state->btn_X = ( ( XB_MASK_X & data[XB_BYTE_BUTTONS_A] ) ? 1 : 0 );
    state->btn_Y = ( ( XB_MASK_Y & data[XB_BYTE_BUTTONS_A] ) ? 1 : 0 );
    state->btn_L_BUMP = ( XB_MASK_L_BUMP & data[XB_BYTE_BUTTONS_A] ? 1 : 0 );
    state->btn_R_BUMP = ( XB_MASK_R_BUMP & data[XB_BYTE_BUTTONS_A] ? 1 : 0 );

    state->joy_L_BTN = ( ( XB_MASK_JOY_L_BTN & data[XB_BYTE_BUTTONS_B] ) ? 1 : 0 );
    state->joy_R_BTN = ( ( XB_MASK_JOY_R_BTN & data[XB_BYTE_BUTTONS_B] ) ? 1 : 0 );
    state->btn_XBOX = ( XB_MASK_XBOX & data[XB_BYTE_BUTTONS_B] ? 1 : 0 );
    state->btn_MENU_LFT = ( XB_MASK_MENU_LFT & data[XB_BYTE_BUTTONS_B] ? 1 : 0 );
    state->btn_MENU_RHT = ( XB_MASK_MENU_RHT & data[XB_BYTE_BUTTONS_B] ? 1 : 0 );

    state->btn_MENU_LOW = ( XB_MASK_MENU_LOW & data[XB_BYTE_MENU_LOW] ? 1 : 0 );

    // Joysticks
    state->joy_L_X = data[XB_BYTE_JOY_L_X];
    state->joy_R_X = data[XB_BYTE_JOY_R_X];
    // Y-axis is inverted
    state->joy_L_Y = XB_JOY_MAX - data[XB_BYTE_JOY_L_Y];
    state->joy_R_Y = XB_JOY_MAX - data[XB_BYTE_JOY_R_Y];

    state->btn_L_TRIG = ( data[XB_BYTE_L_TRIG_HI] << 8 ) | data[XB_BYTE_L_TRIG_LO];
    state->btn_R_TRIG = ( data[XB_BYTE_R_TRIG_HI] << 8 ) | data[XB_BYTE_R_TRIG_LO];

    // static uint32_t i;
    // printf( "Data (Size = %d): ", size );
    // for ( i = 0; i < size; i++ )
    // {
    //    printf( "%02X ", data[i] );
    // }
    // printf( "\n" );
}

void XboxController_BLE::init()
{
    uint8_t request[1] = { 0x41 };
    uint8_t response[3];
    transfer( request, sizeof( request ), response, sizeof( response ) );

    registerBootKeyboardEventHandler( XboxControllerEventHandler, this );

    btstack_main();

    printf( "Xbox Controller BLE initialized!\n" );

    for ( int i = 0; i < 5; ++i )
    {
        DEBUG_BLINK( 100 );
    }
}

void XboxController_BLE::updateState()
{
    uint8_t setRumble = _rumble ? 1 : 0;
    uint8_t request[3] = { 0x40, 0x03, setRumble };
    transfer( request, sizeof( request ), _controllerState, _sizeofControllerState );
}

XboxControllerState_t *XboxController_BLE::getState( void )
{
    return (XboxControllerState_t *)( _controllerState );
}

void XboxController_BLE::getSwitchReport( SwitchReport *switchReport )
{
    // updateState();

    auto state = this->getState();

    // Buttons + Bumpers
    switchReport->buttons[0] =
        ( state->btn_R_TRIG > TRIG_THRESHOLD ? SWITCH_MASK_ZR : 0 ) |
        ( state->btn_R_BUMP ? SWITCH_MASK_R : 0 ) | ( state->btn_A ? SWITCH_MASK_A : 0 ) |
        ( state->btn_B ? SWITCH_MASK_B : 0 ) | ( state->btn_X ? SWITCH_MASK_X : 0 ) |
        ( state->btn_Y ? SWITCH_MASK_Y : 0 );

    // Menu Buttons
    switchReport->buttons[1] = ( state->joy_L_BTN ? SWITCH_MASK_L3 : 0 ) |
                               ( state->joy_R_BTN ? SWITCH_MASK_R3 : 0 ) |
                               ( state->btn_XBOX ? SWITCH_MASK_HOME : 0 ) |
                               ( state->btn_MENU_LFT ? SWITCH_MASK_MINUS : 0 ) |
                               ( state->btn_MENU_RHT ? SWITCH_MASK_PLUS : 0 ) |
                               ( state->btn_MENU_LOW ? SWITCH_MASK_CAPTURE : 0 );

    // D-Pad
    switch ( state->dpad_state )
    {
        case XB_DPAD_UP:
            switchReport->buttons[2] = SWITCH_HAT_UP;
            break;
        case XB_DPAD_UP_RHT:
            switchReport->buttons[2] = SWITCH_HAT_UPRIGHT;
            break;
        case XB_DPAD_RHT:
            switchReport->buttons[2] = SWITCH_HAT_RIGHT;
            break;
        case XB_DPAD_DN_RHT:
            switchReport->buttons[2] = SWITCH_HAT_DOWNRIGHT;
            break;
        case XB_DPAD_DN:
            switchReport->buttons[2] = SWITCH_HAT_DOWN;
            break;
        case XB_DPAD_DN_LFT:
            switchReport->buttons[2] = SWITCH_HAT_DOWNLEFT;
            break;
        case XB_DPAD_LFT:
            switchReport->buttons[2] = SWITCH_HAT_LEFT;
            break;
        case XB_DPAD_UP_LFT:
            switchReport->buttons[2] = SWITCH_HAT_UPLEFT;
            break;
        default:
            switchReport->buttons[2] = SWITCH_HAT_NOTHING;
            break;
    }

    switchReport->buttons[2] |= ( state->btn_L_TRIG > TRIG_THRESHOLD ? SWITCH_MASK_ZL : 0 ) |
                                ( state->btn_L_BUMP ? SWITCH_MASK_L : 0 );

    // Joysticks (convert from Xbox to Switch)
    convertToSwitchJoystick( state->joy_L_X, state->joy_L_Y, switchReport->l );
    convertToSwitchJoystick( state->joy_R_X, state->joy_R_Y, switchReport->r );
}

void XboxController_BLE::convertToSwitchJoystick( uint16_t x_axis, uint16_t y_axis, uint8_t *dst )
{
    // Convert to the format [ 00 YY YX XX ]
    uint32_t pos = ( ( XB2SW_JOY( y_axis, XB_JOY_MAX, SW_JOY_MAX ) & SW_JOY_MAX ) << 12 ) |
                   ( ( XB2SW_JOY( x_axis, XB_JOY_MAX, SW_JOY_MAX ) & SW_JOY_MAX ) );

    // Reverse the byte order
    dst[0] = ( pos >> 0x00 );
    dst[1] = ( pos >> 0x08 );
    dst[2] = ( pos >> 0x10 );
}
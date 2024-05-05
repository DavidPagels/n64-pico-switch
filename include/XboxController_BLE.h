#ifndef XBOX_CONTROLLER_BLE_H
#define XBOX_CONTROLLER_BLE_H

#include "Controller.h"

#define XB_JOY_MIN  ( 0x00U )
#define XB_JOY_MAX  ( 0xFFU )
#define XB_TRIG_MIN ( 0x000U )
#define XB_TRIG_MAX ( 0x3FFU )

// Joysticks
#define XB_BYTE_JOY_L_X ( 0x01U )
#define XB_BYTE_JOY_L_Y ( 0x03U )
#define XB_BYTE_JOY_R_X ( 0x05U )
#define XB_BYTE_JOY_R_Y ( 0x07U )
// Triggers
#define XB_BYTE_L_TRIG_LO ( 0x08 )
#define XB_BYTE_L_TRIG_HI ( 0x09 )
#define XB_BYTE_R_TRIG_LO ( 0x0A )
#define XB_BYTE_R_TRIG_HI ( 0x0B )
// D-Pad
#define XB_BYTE_DPAD   ( 0x0CU )
#define XB_DPAD_UP     ( 0x01U )
#define XB_DPAD_UP_RHT ( 0x02U )
#define XB_DPAD_RHT    ( 0x03U )
#define XB_DPAD_DN_RHT ( 0x04U )
#define XB_DPAD_DN     ( 0x05U )
#define XB_DPAD_DN_LFT ( 0x06U )
#define XB_DPAD_LFT    ( 0x07U )
#define XB_DPAD_UP_LFT ( 0x08U )
// Buttons + Bumpers
#define XB_BYTE_BUTTONS_A ( 0x0DU )
#define XB_MASK_A         ( 0x01U )
#define XB_MASK_B         ( 0x02U )
#define XB_MASK_X         ( 0x08U )
#define XB_MASK_Y         ( 0x10U )
#define XB_MASK_L_BUMP    ( 0x40U )
#define XB_MASK_R_BUMP    ( 0x80U )
// Menu + Joystick Buttons
#define XB_BYTE_BUTTONS_B ( 0x0EU )
#define XB_MASK_MENU_LFT  ( 0x04U )
#define XB_MASK_MENU_RHT  ( 0x08U )
#define XB_MASK_XBOX      ( 0x10U )
#define XB_MASK_JOY_L_BTN ( 0x20U )
#define XB_MASK_JOY_R_BTN ( 0x40U )
// Lower Menu Button
#define XB_BYTE_MENU_LOW ( 0x0FU )
#define XB_MASK_MENU_LOW ( 0x01U )

typedef struct XboxControllerState
{
    // Buttons
    uint8_t btn_A;
    uint8_t btn_B;
    uint8_t btn_X;
    uint8_t btn_Y;
    // D-Pad
    uint8_t dpad_state;  // NEUTRAL = 0, UP = 1, increment by 1 for each clockwise position
    // Joysticks
    uint8_t joy_L_X;
    uint8_t joy_L_Y;
    uint8_t joy_L_BTN;
    uint8_t joy_R_X;
    uint8_t joy_R_Y;
    uint8_t joy_R_BTN;
    // Triggers
    uint8_t btn_L_BUMP;
    uint8_t btn_R_BUMP;
    uint8_t btn_L_TRIG;
    uint8_t btn_R_TRIG;
    // Menu
    uint8_t btn_XBOX;
    uint8_t btn_MENU_LFT;  // Button with two square symbols
    uint8_t btn_MENU_RHT;  // Button with three lines
    uint8_t btn_MENU_LOW;  // Button with arrow pointing up (below the other two menu buttons)

} XboxControllerState_t;

class XboxController_BLE : public Controller {
   public:
    XboxController_BLE( InitParams *initParams )
        : Controller( initParams, sizeof( XboxControllerState_t ) ){};
    void init();
    void getSwitchReport( SwitchReport *switchReport );
    void setRumble( bool rumble ) { _rumble = rumble; };
    XboxControllerState_t *getState( void );

   private:
    void updateState();
    // void XboxControllerEventHandler( const uint8_t *data, uint16_t size );
    void convertToSwitchJoystick( uint16_t x_axis, uint16_t y_axis, uint8_t *dst );
};

#endif /* XBOX_CONTROLLER_BLE_H */
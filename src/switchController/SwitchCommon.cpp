#include "SwitchCommon.h"

#include <stdlib.h>
#include <string.h>

void SwitchCommon::setSwitchRequestReport( uint8_t *report, int report_size )
{
    memcpy( _switchRequestReport, report, report_size );
}

uint8_t *SwitchCommon::generate_report()
{
    set_empty_report();
    _report[0] = 0xa1;
    switch ( _switchRequestReport[10] )
    {
        case BLUETOOTH_PAIR_REQUEST:
            set_subcommand_reply();
            set_bt();
            break;
        case REQUEST_DEVICE_INFO:
            _device_info_queried = true;
            set_subcommand_reply();
            set_device_info();
            break;
        case SET_SHIPMENT:
            set_subcommand_reply();
            set_shipment();
            break;
        case SPI_READ:
            set_subcommand_reply();
            spi_read();
            break;
        case SET_MODE:
            set_subcommand_reply();
            set_mode();
            break;
        case TRIGGER_BUTTONS:
            set_subcommand_reply();
            set_trigger_buttons();
            break;
        case TOGGLE_IMU:
            set_subcommand_reply();
            toggle_imu();
            break;
        case IMU_SENSITIVITY:
            set_subcommand_reply();
            imu_sensitivity();
            break;
        case ENABLE_VIBRATION:
            set_subcommand_reply();
            enable_vibration();
            break;
        case SET_PLAYER:
            set_subcommand_reply();
            set_player_lights();
            break;
        case SET_NFC_IR_STATE:
            set_subcommand_reply();
            set_nfc_ir_state();
            break;
        case SET_NFC_IR_CONFIG:
            set_subcommand_reply();
            set_nfc_ir_config();
            break;
        default:
            set_full_input_report();
            break;
    }
    return _report;
}

void SwitchCommon::set_empty_report() { memset( _report, 0x00, sizeof( _report ) ); }

void SwitchCommon::set_empty_switch_request_report()
{
    memset( _switchRequestReport, 0x00, sizeof( _switchRequestReport ) );
}

void SwitchCommon::set_subcommand_reply()
{
    // Input Report ID
    _report[1] = 0x21;

    // TODO: Find out what the vibrator byte is doing.
    // This is a hack in an attempt to semi-emulate
    // actions of the vibrator byte as it seems to change
    // when a subcommand reply is sent.
    if ( _vibration_enabled )
    {
        _vibration_idx = ( _vibration_idx + 1 ) % 4;
        _vibration_report = VIB_OPTS[_vibration_idx];
    }

    set_standard_input_report();
}

void SwitchCommon::set_unknown_subcommand( uint8_t subcommand_id )
{
    // Set NACK
    _report[14];

    // Set unknown subcommand ID
    _report[15] = subcommand_id;
}

void SwitchCommon::set_timer()
{
    // If the timer hasn't been set before
    if ( _timestamp == 0 )
    {
        _timestamp = to_ms_since_boot( get_absolute_time() );
        _report[2] = 0x00;
        return;
    }

    // Get the time that has passed since the last timestamp
    // in milliseconds
    uint32_t now = to_ms_since_boot( get_absolute_time() );
    uint32_t delta_t = ( now - _timestamp );

    // Get how many ticks have passed in hex with overflow at 255
    // Joy-Con uses 4.96ms as the timer tick rate
    uint32_t elapsed_ticks = int( delta_t * 4 );
    _timer = ( _timer + elapsed_ticks ) & 0xFF;

    _report[2] = _timer;
    _timestamp = now;
}

void SwitchCommon::set_full_input_report()
{
    // Setting Report ID to full standard input report ID
    _report[1] = 0x30;

    set_standard_input_report();
    set_imu_data();
}

void SwitchCommon::set_standard_input_report()
{
    set_timer();

    _controller->getSwitchReport( &_switchReport );
    memcpy( _report + 3, (uint8_t *)&_switchReport, sizeof( SwitchReport ) );
    _report[13] = _vibration_report;
}

void SwitchCommon::set_bt()
{
    _report[14] = 0x81;
    _report[15] = 0x01;
    _report[16] = 0x03;
}

void SwitchCommon::set_device_info()
{
    // ACK Reply
    _report[14] = 0x82;

    // Subcommand Reply
    _report[15] = 0x02;

    // Firmware version
    _report[16] = 0x03;
    _report[17] = 0x48;

    // Controller ID
    _report[18] = 0x03;

    // Unknown Byte, always 2
    _report[19] = 0x02;

    // Controller Bluetooth Address
    memcpy( _report + 20, _addr, 6 );

    // Unknown byte, always 1
    _report[26] = 0x01;

    // Controller colours location (read from SPI)
    _report[27] = 0x01;
}

void SwitchCommon::set_shipment()
{
    // ACK Reply
    _report[14] = 0x80;

    // Subcommand reply
    _report[15] = 0x08;
}

void SwitchCommon::toggle_imu()
{
    _imu_enabled = _switchRequestReport[11] == 0x01;

    // ACK Reply
    _report[14] = 0x80;

    // Subcommand reply
    _report[15] = 0x40;
}

void SwitchCommon::imu_sensitivity()
{
    _report[14] = 0x80;
    _report[15] = 0x41;
}

void SwitchCommon::set_imu_data()
{
    if ( !_imu_enabled )
    {
        return;
    }

    uint8_t imu_data[49] = { 0x75, 0xFD, 0xFD, 0xFF, 0x09, 0x10, 0x21, 0x00, 0xD5,
                             0xFF, 0xE0, 0xFF, 0x72, 0xFD, 0xF9, 0xFF, 0x0A, 0x10,
                             0x22, 0x00, 0xD5, 0xFF, 0xE0, 0xFF, 0x76, 0xFD, 0xFC,
                             0xFF, 0x09, 0x10, 0x23, 0x00, 0xD5, 0xFF, 0xE0, 0xFF };
    memcpy( _report + 14, imu_data, sizeof( imu_data ) );
}
void SwitchCommon::spi_read()
{
    uint8_t addr_top = _switchRequestReport[12];
    uint8_t addr_bottom = _switchRequestReport[11];
    uint8_t read_length = _switchRequestReport[15];

    // ACK byte
    _report[14] = 0x90;

    // Subcommand reply
    _report[15] = 0x10;

    // Read address
    _report[16] = addr_bottom;
    _report[17] = addr_top;

    // Read length
    _report[20] = read_length;

    // Stick Parameters
    // Params are generally the same for all sticks
    // Notable difference is the deadzone (10%
    // Joy-Con vs 15% Pro Con)
    uint8_t params[18] = { 0x0F, 0x30, 0x61,  // Unused
                           0x96, 0x30,
                           0xF3,                // Dead Zone/Range Ratio
                           0xD4, 0x14, 0x54,    // X/Y ?
                           0x41, 0x15, 0x54,    // X/Y ?
                           0xC7, 0x79, 0x9C,    // X/Y ?
                           0x33, 0x36, 0x63 };  // X/Y ?

    // Serial Number read
    if ( addr_top == 0x60 && addr_bottom == 0x00 )
    {
        // Switch will take this as no serial number
        memset( _report + 21, 0xff, 16 );
    }
    else if ( addr_top == 0x60 && addr_bottom == 0x50 )
    {
        // Body colour
        memset( _report + 21, 0x32, 3 );
        // Buttons colour
        memset( _report + 24, 0xff, 3 );
        // Left/right grip colours (Pro controller)
        memset( _report + 27, 0xff, 7 );
    }
    else if ( addr_top == 0x60 && addr_bottom == 0x80 )
    {
        // Six-Axis factory parameters
        _report[21] = 0x50;
        _report[22] = 0xFD;
        _report[23] = 0x00;
        _report[24] = 0x00;
        _report[25] = 0xC6;
        _report[26] = 0x0F;

        memcpy( _report + 27, params, sizeof( params ) );

        // Stick device parameters 2
    }
    else if ( addr_top == 0x60 && addr_bottom == 0x98 )
    {
        // Setting same params since controllers always
        // have duplicates of stick params 1 for stick params 2
        memcpy( _report + 21, params, sizeof( params ) );

        // User analog stick calibration
    }
    else if ( addr_top == 0x80 && addr_bottom == 0x10 )
    {
        // Fill report with null user calibration info
        memset( _report + 21, 0xff, 3 );

        // Factory analog stick calibration
    }
    else if ( addr_top == 0x60 && addr_bottom == 0x3D )
    {
        // Left/right stick calibration
        uint8_t l_calibration[9] = { 0xD4, 0x75, 0x61, 0xE5, 0x87, 0x7C, 0xEC, 0x55, 0x61 };
        uint8_t r_calibration[9] = { 0x5D, 0xD8, 0x7F, 0x18, 0xE6, 0x61, 0x86, 0x65, 0x5D };

        // Left stick calibration

        memcpy( _report + 21, l_calibration, sizeof( l_calibration ) );

        // Right stick calibration
        memcpy( _report + 30, r_calibration, sizeof( r_calibration ) );

        // Spacer byte
        _report[39] = 0xFF;

        // Body colour
        memset( _report + 40, 0x32, 3 );
        // Buttons colour
        memset( _report + 43, 0xff, 3 );

        // Six-Axis motion sensor factor
        // calibration
    }
    else if ( addr_top == 0x60 && addr_bottom == 0x20 )
    {
        // 1: Acceleration origin position
        // 2: Acceleration sensitivity coefficient
        // 3: Gyro origin when still
        // 4: Gyro sensitivity coefficient
        uint8_t sa_calibration[24] = { 0xcc, 0x00, 0x40, 0x00, 0x91, 0x01,    // 1
                                       0x00, 0x40, 0x00, 0x40, 0x00, 0x40,    // 2
                                       0xe7, 0xff, 0x0e, 0x00, 0xdc, 0xff,    // 3
                                       0x3b, 0x34, 0x3b, 0x34, 0x3b, 0x34 };  // 4

        memcpy( _report + 21, sa_calibration, sizeof( sa_calibration ) );
    }
    else
    {
        memset( _report + 21, 0xFF, read_length );
    }
}

void SwitchCommon::set_mode()
{
    // ACK byte
    _report[14] = 0x80;

    // Subcommand reply
    _report[15] = 0x03;
}

void SwitchCommon::set_trigger_buttons()
{
    // ACK byte
    _report[14] = 0x83;

    // Subcommand reply
    _report[15] = 0x04;
}

void SwitchCommon::enable_vibration()
{
    // ACK Reply
    _report[14] = 0x80;

    // Subcommand reply
    _report[15] = 0x48;

    // Set class property
    _vibration_enabled = true;
    _vibration_idx = 0;
    _vibration_report = VIB_OPTS[_vibration_idx];
}

void SwitchCommon::set_player_lights()
{
    // ACK byte
    _report[14] = 0x80;

    // Subcommand reply
    _report[15] = 0x30;

    uint8_t bitfield = _switchRequestReport[11];

    if ( bitfield == 0x01 || bitfield == 0x10 )
    {
        _player_number = 1;
    }
    else if ( bitfield == 0x03 || bitfield == 0x30 )
    {
        _player_number = 2;
    }
    else if ( bitfield == 0x07 || bitfield == 0x70 )
    {
        _player_number = 3;
    }
    else if ( bitfield == 0x0F || bitfield == 0xF0 )
    {
        _player_number = 4;
    }
}

void SwitchCommon::set_nfc_ir_state()
{
    // ACK byte
    _report[14] = 0x80;

    // Subcommand reply
    _report[15] = 0x22;
}

void SwitchCommon::set_nfc_ir_config()
{
    // ACK byte
    _report[14] = 0xA0;

    // Subcommand reply
    _report[15] = 0x21;

    // NFC/IR state data
    uint8_t params[8] = { 0x01, 0x00, 0xFF, 0x00, 0x08, 0x00, 0x1B, 0x01 };
    memcpy( _report + 16, params, sizeof( params ) );
    _report[49] = 0xC8;
}

void SwitchCommon::set_controller_rumble( bool rumble ) { _controller->setRumble( rumble ); }

void hid_report_data_callback(
    SwitchCommon *inst, uint16_t report_id, uint8_t *report, int report_size
)
{
    if ( report_id == 0x01 || report_id == 0x10 || report_id == 0x11 )
    {
        bool lValid = ( report[2] & 0x03 ) == 0x00 && ( report[5] & 0x40 ) == 0x40;
        bool rValid = ( report[6] & 0x03 ) == 0x00 && ( report[9] & 0x40 ) == 0x40;
        if ( lValid || rValid )
        {
            bool rumbling = ( ( lValid ? ( report[5] & 0x3F ) : 0x00 ) |
                              ( rValid ? ( report[9] & 0x3F ) : 0x00 ) ) > 0x02;
            inst->set_controller_rumble( rumbling );
        }
    }
    inst->setSwitchRequestReport( report, report_size );
}

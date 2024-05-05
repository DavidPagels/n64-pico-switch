/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "SwitchUsb.h"

#include "pico/rand.h"
#include "tusb.h"

static uint16_t _desc_str[32];

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const *tud_descriptor_device_cb( void ) { return switch_usb_device_descriptor; }

uint8_t *SwitchUsb::generate_usb_report()
{
    set_empty_report();

    if ( _switchRequestReport[0] == 0x80 )
    {
        _report[0] = 0x81;
        _report[1] = _switchRequestReport[1];
        switch ( _switchRequestReport[1] )
        {
            case 0x01:
                _report[3] = 0x03;
                for ( int i = 0; i < 6; i++ )
                {
                    _report[4 + i] = _addr[5 - i];
                }
                break;
            case 0x02:
            case 0x03:
                break;
            default:
                _report[0] = 0x30;
                _controller->getSwitchReport( &_switchReport );
                memcpy( _report + 2, (uint8_t *)&_switchReport, sizeof( SwitchReport ) );
                break;
        }
        if ( _switchRequestReport[0] > 0x00 )
        {
            set_empty_switch_request_report();
        }
        return _report;
    }
    else
    {
        generate_report();
        if ( _switchRequestReport[0] > 0x00 )
        {
            set_empty_switch_request_report();
        }
        // _report is a bluetooth report starting with 0xA1, which usb skips
        return _report + 1;
    }
};

void SwitchUsb::init( Controller *controller )
{
    _controller = controller;
    _switchReport.batteryConnection = 0x81;
    _switchRequestReport[0] = 0x80;
    _switchRequestReport[1] = 0x01;
    uint8_t newAddr[] = {
        0x7c,
        0xbb,
        0x8a,
        (uint8_t)( get_rand_32() % 0xff ),
        (uint8_t)( get_rand_32() % 0xff ),
        (uint8_t)( get_rand_32() % 0xff )
    };
    memcpy( _addr, newAddr, 6 );
    tusb_init();
    while ( true )
    {
        tud_task();
        try
        {
            tud_task();
            if ( tud_suspended() )
            {
                tud_remote_wakeup();
            }
            if ( tud_hid_ready() )
            {
                uint8_t *report = generate_usb_report();
                tud_hid_report( 0, report, 64 );
            }
        } catch ( int e )
        {
            tud_task();
            if ( tud_suspended() )
            {
                tud_remote_wakeup();
            }
        }
    }
}

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long
// enough for transfer to complete
uint16_t const *tud_descriptor_string_cb( uint8_t index, uint16_t langid )
{
    (void)langid;

    uint8_t chr_count;

    if ( index == 0 )
    {
        memcpy( &_desc_str[1], switch_usb_string_descriptors[0], 2 );
        chr_count = 1;
    }
    else
    {
        // Note: the 0xEE index string is a Microsoft OS 1.0 Descriptors.
        // https://docs.microsoft.com/en-us/windows-hardware/drivers/usbcon/microsoft-defined-usb-descriptors

        if ( !( index < sizeof( switch_usb_string_descriptors ) /
                            sizeof( switch_usb_string_descriptors[0] ) ) )
            return NULL;

        const char *str = (char *)switch_usb_string_descriptors[index];

        // Cap at max char
        chr_count = (uint8_t)strlen( str );
        if ( chr_count > 31 ) chr_count = 31;

        // Convert ASCII string into UTF-16
        for ( uint8_t i = 0; i < chr_count; i++ )
        {
            _desc_str[1 + i] = str[i];
        }
    }

    // first byte is length (including header), second byte is string type
    _desc_str[0] = (uint16_t)( ( 0x03 << 8 ) | ( 2 * chr_count + 2 ) );

    return _desc_str;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_hid_descriptor_report_cb( uint8_t itf )
{
    (void)itf;
    return switch_usb_report_descriptor;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const *tud_descriptor_configuration_cb( uint8_t index )
{
    (void)index;  // for multiple configurations
    return switch_usb_configuration_descriptor;
}

uint16_t tud_hid_get_report_cb(
    uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen
)
{
    return reqlen;
}

#ifndef HOG_HOST_H
#define HOG_HOST_H

// Include necessary dependencies for the Pico

#include <stdio.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void ( *BootKeyboardEventHandler )( void* ctx, const uint8_t* data, uint16_t size );

void registerBootKeyboardEventHandler( BootKeyboardEventHandler callback, void* context );

int btstack_main( void );

#ifdef __cplusplus
}
#endif

#endif /* HOG_HOST_H */
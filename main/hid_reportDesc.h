#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

const uint8_t * get_hidDescriptor( void );
uint16_t get_hidDescriptorLen( void );

#ifdef __cplusplus
}
#endif

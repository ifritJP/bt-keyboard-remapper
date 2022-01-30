#include <stdint.h>
#include <HID.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  extern void my_kbd_init( void );
  extern void my_kbd_conv( const uint8_t * pSrcReport, int length,
			   HIDReport_t * pReport );
  extern void my_kbd_dump( void );
  extern bool my_kbd_add_key( uint8_t modifier, uint8_t keycode );
  extern bool my_kbd_add_report( const HIDReport_t * pReport );
  extern bool my_kbd_get_report( HIDReport_t * pReport );
  extern bool my_kbd_has_report( void );
  extern void my_kbd_setHidRemap( uint8_t oldCode, uint8_t newCode );
  extern void my_kbd_SetConvKeyInfo( uint8_t code, const ConvKeyInfo_t * pInfo );
  extern void my_kbd_save( void );
  extern void my_kbd_load( void );
  extern void my_kbd_clear(void );
  extern void my_kbd_dumpBase64( void );
  extern void my_kbd_readBase64( void );


#ifdef __cplusplus
}
#endif

#include <stdint.h>
#include <HID.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define REMAP_CODE_LEN 256

  typedef struct {
    // HID の remap コード。
    // index: 変換元 HID CODE → val: 変換先 HID CODE
    uint8_t  remapHIDCode[ REMAP_CODE_LEN ];
  } Code2HidCode_t;

  extern Code2HidCode_t newCode2HidCode( void );
  extern void InitHidRemap( Code2HidCode_t * pConv );
  extern void SetHIDRemap( Code2HidCode_t * pConv,  uint8_t oldCode, uint8_t newCode );
  extern void ProcessHIDReport( Code2HidCode_t * pConv,
				HIDKeyboard_t * pKeyboard, const HIDReport_t * pReport );
  extern void DumpRemap( const Code2HidCode_t * pConv );

#ifdef __cplusplus
}
#endif
  

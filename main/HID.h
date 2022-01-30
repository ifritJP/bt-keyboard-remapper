#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

  typedef struct {
    // HID の modifier の一致条件。
    // HID の modifier と以下の式が成立する時に、このキーに置き換える。
    // (modifier & condModifierMask) == condModifierResult
    uint8_t CondModifierMask;
    uint8_t CondModifierResult;
    // HID コード
    uint8_t Code;
    // modifier に XOR する値
    uint8_t ModifierXor;
  } ConvKeyInfo_t;

  // HID のキーの状態
  typedef struct {
    // HID キーコード
    uint8_t OrgCode;
    // キーコード名
    const char * string;
    // modifier キーかどうか
    bool IsModifier;
  } HIDKeyInfoConst_t;


  // HID のキーの状態
  typedef struct {
    const HIDKeyInfoConst_t * pConst;
    // 押されているかどうか
    bool Pressed;
    uint8_t convKeyInfoLen;
    // ConvKeyInfo
    const ConvKeyInfo_t * convKeyInfoList;
  } HIDKeyInfo_t;

#define HID_REPORT_LEN 8
  typedef struct {
    uint8_t codes[ HID_REPORT_LEN ];
  } HIDReport_t;


#define HID_KEY_NUM 256

  typedef struct {
    // HID のキーパケット 8 バイト
    // byte0: modifier
    // byte1: reservede
    // byte2: pressed-key1
    // byte3: pressed-key2
    // byte4: pressed-key3
    // byte5: pressed-key4
    // byte6: pressed-key5
    // byte7: pressed-key6
    uint8_t data[ HID_REPORT_LEN ];
    // index: HID キーコード → val: HIDKeyInfo
    HIDKeyInfo_t keyInfoMap[ HID_KEY_NUM ];
  } HIDKeyboard_t ;


  extern HIDKeyboard_t * NewHIDKeyboard( void );
  extern void HID_ClearHIDKeyboard( HIDKeyboard_t * pKeyboard );
  extern void HID_PressKey( HIDKeyboard_t * pKeyboard, uint8_t code );
  extern void HID_ReleaseKey( HIDKeyboard_t * pKeyboard, uint8_t code );
  extern void HID_ReleaseAllKeys( HIDKeyboard_t * pKeyboard );
  extern const HIDKeyInfo_t * HID_GetKeyInfo( HIDKeyboard_t * pKeyboard, uint8_t code );
  extern void HID_SetConvKeyInfo( HIDKeyboard_t * pKeyboard, uint8_t code, const ConvKeyInfo_t * pList, int len);
  extern void HID_SetupHidPackat( HIDKeyboard_t * pKeyboard, HIDReport_t * pReport );
  extern void HID_dumpConvKeyInfo( HIDKeyboard_t * pKeyboard );


#ifdef __cplusplus
}
#endif
  

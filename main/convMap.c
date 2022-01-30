#include <convMap.h>
#include <stdio.h>

Code2HidCode_t newCode2HidCode( void ) {
    int index;
    Code2HidCode_t code2HidCode;
    InitHidRemap( &code2HidCode );
    return code2HidCode;
}

void InitHidRemap( Code2HidCode_t * pConv ) {
    int index;
    for ( index = 0; index < REMAP_CODE_LEN; index++ ) {
        pConv->remapHIDCode[ index ] = index;
    }
}

void SetHIDRemap( Code2HidCode_t * pConv, uint8_t oldCode, uint8_t newCode ) {
    pConv->remapHIDCode[ oldCode ] = newCode;
}

void ProcessHIDReport( Code2HidCode_t * pConv,
                       HIDKeyboard_t * pKeyboard, const HIDReport_t * pReport )
{
    int index = 0;
    uint8_t modifierFlag = pReport->codes[ 0 ];
    for ( index = 0; index < 8; index++ ) {
        uint8_t mask = 1 << index;
        uint8_t code = 0xE0 + index;
        code = pConv->remapHIDCode[ code ];
        if ( modifierFlag & mask ) {
            HID_PressKey( pKeyboard, code );
        } else {
            HID_ReleaseKey( pKeyboard, code );
        }
    }
    for ( index = 2; index < HID_REPORT_LEN; index++ ) {
        uint8_t code = pReport->codes[ index ];
        code = pConv->remapHIDCode[ code ];
        if ( code >= 4 ) {
            HID_PressKey( pKeyboard, code );
        }
    }
}

void DumpRemap( const Code2HidCode_t * pConv ) {
    int index;
    for ( index = 0; index < REMAP_CODE_LEN; index++ ) {
        uint8_t code = pConv->remapHIDCode[ index ];
        if ( index != code ) {
            printf( "0x%02X -> 0x%02X\n", index, code );
        }
    }
}

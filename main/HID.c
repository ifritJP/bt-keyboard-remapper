#include <stdint.h>
#include <HID.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


// HID の modifier bit を返す
static uint8_t GetModifierBit( const HIDKeyInfo_t * pInfo ) {
    if ( pInfo->Pressed && pInfo->pConst->IsModifier ) {
        return 1 << ( pInfo->pConst->OrgCode - 0xe0 );
    }
    return 0;
}

// 置き換えを処理する。
//
// @param modifierFlag modifierFlag
// @return byte 置き換え後の HID キーコード
static uint8_t HID_process( HIDKeyInfo_t * pInfo, uint8_t * pModifierFlag ) {
    int index;
    for ( index = 0; index < pInfo->convKeyInfoLen; index++ ) {
	const ConvKeyInfo_t * pConvKey = &pInfo->convKeyInfoList[ index ];
        // 置き換え情報を処理する
        uint8_t mask = *pModifierFlag & pConvKey->CondModifierMask;
        if ( mask == pConvKey->CondModifierResult ) {
            *pModifierFlag = *pModifierFlag ^ pConvKey->ModifierXor;
            return pConvKey->Code;
        }
    }
    if  ( !pInfo->pConst->IsModifier ) {
        return pInfo->pConst->OrgCode;
    }
    return 0;
}


static const HIDKeyInfoConst_t s_HIDKeyInfoConst[] = {
    { 0x00, "Reserved", false},
    { 0x01, "Kb ErrorRollOver", false},
    { 0x02, "Kb POSTFail", false},
    { 0x03, "Kb ErrorUndefined", false},
    { 0x04, "Kb a", false},
    { 0x05, "Kb b", false},
    { 0x06, "Kb c", false},
    { 0x07, "Kb d", false},
    { 0x08, "Kb e", false},
    { 0x09, "Kb f", false},
    { 0x0A, "Kb g", false},
    { 0x0B, "Kb h", false},
    { 0x0C, "Kb i", false},
    { 0x0D, "Kb j", false},
    { 0x0E, "Kb k", false},
    { 0x0F, "Kb l", false},
    { 0x10, "Kb m", false},
    { 0x11, "Kb n", false},
    { 0x12, "Kb o", false},
    { 0x13, "Kb p", false},
    { 0x14, "Kb q", false},
    { 0x15, "Kb r", false},
    { 0x16, "Kb s", false},
    { 0x17, "Kb t", false},
    { 0x18, "Kb u", false},
    { 0x19, "Kb v", false},
    { 0x1A, "Kb w", false},
    { 0x1B, "Kb x", false},
    { 0x1C, "Kb y", false},
    { 0x1D, "Kb z", false},
    { 0x1E, "Kb 1 and !", false},
    { 0x1F, "Kb 2 and @", false},
    { 0x20, "Kb 3 and #", false},
    { 0x21, "Kb 4 and $", false},
    { 0x22, "Kb 5 and %", false},
    { 0x23, "Kb 6 and ∧", false},
    { 0x24, "Kb 7 and &", false},
    { 0x25, "Kb 8 and *", false},
    { 0x26, "Kb 9 and (", false},
    { 0x27, "Kb 0 and )", false},
    { 0x28, "Kb Return (ENTER)", false},
    { 0x29, "Kb ESCAPE", false},
    { 0x2A, "Kb DELETE (Backspace)", false},
    { 0x2B, "Kb Tab", false},
    { 0x2C, "Kb Spacebar", false},
    { 0x2D, "Kb - and (underscore)", false},
    { 0x2E, "Kb = and +", false},
    { 0x2F, "Kb [ and {", false},
    { 0x30, "Kb ] and }", false},
    { 0x31, "Kb \\ and |", false},
    { 0x32, "Kb Non-US # and `", false},
    { 0x33, "Kb ; and :", false},
    { 0x34, "Kb ' and \"", false},
    { 0x35, "Kb Grave Accent and Tilde", false},
    { 0x36, "Kb , and <", false},
    { 0x37, "Kb . and >", false},
    { 0x38, "Kb / and ?", false},
    { 0x39, "Kb Caps Lock", false},
    { 0x3A, "Kb F1", false},
    { 0x3B, "Kb F2", false},
    { 0x3C, "Kb F3", false},
    { 0x3D, "Kb F4", false},
    { 0x3E, "Kb F5", false},
    { 0x3F, "Kb F6", false},
    { 0x40, "Kb F7", false},
    { 0x41, "Kb F8", false},
    { 0x42, "Kb F9", false},
    { 0x43, "Kb F10", false},
    { 0x44, "Kb F11", false},
    { 0x45, "Kb F12", false},
    { 0x46, "Kb PrintScreen", false},
    { 0x47, "Kb Scroll Lock", false},
    { 0x48, "Kb Pause", false},
    { 0x49, "Kb Insert", false},
    { 0x4A, "Kb Home", false},
    { 0x4B, "Kb PageUp", false},
    { 0x4C, "Kb Delete Forward", false},
    { 0x4D, "Kb End", false},
    { 0x4E, "Kb PageDown", false},
    { 0x4F, "Kb RightArrow", false},
    { 0x50, "Kb LeftArrow", false},
    { 0x51, "Kb DownArrow", false},
    { 0x52, "Kb UpArrow", false},
    { 0x53, "Kpd Num Lock and Clear", false},
    { 0x54, "Kpd /", false},
    { 0x55, "Kpd *", false},
    { 0x56, "Kpd -", false},
    { 0x57, "Kpd +", false},
    { 0x58, "Kpd ENTER", false},
    { 0x59, "Kpd 1 and End", false},
    { 0x5A, "Kpd 2 and Down Arrow", false},
    { 0x5B, "Kpd 3 and PageDn", false},
    { 0x5C, "Kpd 4 and Left Arrow", false},
    { 0x5D, "Kpd 5", false},
    { 0x5E, "Kpd 6 and Right Arrow", false},
    { 0x5F, "Kpd 7 and Home", false},
    { 0x60, "Kpd 8 and Up Arrow", false},
    { 0x61, "Kpd 9 and PageUp", false},
    { 0x62, "Kpd 0 and Insert", false},
    { 0x63, "Kpd . and Delete", false},
    { 0x64, "Kb Non-US \\ and |", false},
    { 0x65, "Kb Application", false},
    { 0x66, "Kb Power", false},
    { 0x67, "Kpd =", false},
    { 0x68, "Kb F13", false},
    { 0x69, "Kb F14", false},
    { 0x6A, "Kb F15", false},
    { 0x6B, "Kb F16 ", false},
    { 0x6C, "Kb F17 ", false},
    { 0x6D, "Kb F18 ", false},
    { 0x6E, "Kb F19 ", false},
    { 0x6F, "Kb F20 ", false},
    { 0x70, "Kb F21 ", false},
    { 0x71, "Kb F22 ", false},
    { 0x72, "Kb F23 ", false},
    { 0x73, "Kb F24 ", false},
    { 0x74, "Kb Execute", false},
    { 0x75, "Kb Help", false},
    { 0x76, "Kb Menu", false},
    { 0x77, "Kb Select", false},
    { 0x78, "Kb Stop", false},
    { 0x79, "Kb Again", false},
    { 0x7A, "Kb Undo", false},
    { 0x7B, "Kb Cut", false},
    { 0x7C, "Kb Copy", false},
    { 0x7D, "Kb Paste", false},
    { 0x7E, "Kb Find", false},
    { 0x7F, "Kb Mute", false},
    { 0x80, "Kb Volume Up", false},
    { 0x81, "Kb Volume Down", false},
    { 0x82, "Kb Locking Caps Lock", false},
    { 0x83, "Kb Locking Num Lock", false},
    { 0x84, "Kb Locking Scroll Lock", false},
    { 0x85, "Kpd Comma", false},
    { 0x86, "Kpd Equal Sign", false},
    { 0x87, "Kb International1", false},
    { 0x88, "Kb International2 katakana-hiragana", false},
    { 0x89, "Kb International3 ", false},
    { 0x8A, "Kb International4 henkan", false},
    { 0x8B, "Kb International5 muhenkan", false},
    { 0x8C, "Kb International6 ", false},
    { 0x8D, "Kb International7 ", false},
    { 0x8E, "Kb International8 ", false},
    { 0x8F, "Kb International9 ", false},
    { 0x90, "Kb LANG1", false},
    { 0x91, "Kb LANG2", false},
    { 0x92, "Kb LANG3", false},
    { 0x93, "Kb LANG4", false},
    { 0x94, "Kb LANG5", false},
    { 0x95, "Kb LANG6", false},
    { 0x96, "Kb LANG7", false},
    { 0x97, "Kb LANG8", false},
    { 0x98, "Kb LANG9", false},
    { 0x99, "Kb Alternate Erase", false},
    { 0x9A, "Kb SysReq/Attention", false},
    { 0x9B, "Kb Cancel ", false},
    { 0x9C, "Kb Clear ", false},
    { 0x9D, "Kb Prior ", false},
    { 0x9E, "Kb Return ", false},
    { 0x9F, "Kb Separator ", false},
    { 0xA0, "Kb Out ", false},
    { 0xA1, "Kb Oper ", false},
    { 0xA2, "Kb Clear/Again ", false},
    { 0xA3, "Kb CrSel/Props ", false},
    { 0xA4, "Kb ExSel ", false},
    { 0xB0, "Kpd 00 ", false},
    { 0xB1, "Kpd 000 ", false},
    { 0xB2, "Thousands Separator", false},
    { 0xB3, "Decimal Separator", false},
    { 0xB4, "Currency Unit", false},
    { 0xB5, "Currency Sub-unit", false},
    { 0xB6, "Kpd ( ", false},
    { 0xB7, "Kpd ) ", false},
    { 0xB8, "Kpd { ", false},
    { 0xB9, "Kpd } ", false},
    { 0xBA, "Kpd Tab ", false},
    { 0xBB, "Kpd Backspace ", false},
    { 0xBC, "Kpd A ", false},
    { 0xBD, "Kpd B ", false},
    { 0xBE, "Kpd C ", false},
    { 0xBF, "Kpd D ", false},
    { 0xC0, "Kpd E ", false},
    { 0xC1, "Kpd F ", false},
    { 0xC2, "Kpd XOR ", false},
    { 0xC3, "Kpd ∧ ", false},
    { 0xC4, "Kpd % ", false},
    { 0xC5, "Kpd < ", false},
    { 0xC6, "Kpd > ", false},
    { 0xC7, "Kpd & ", false},
    { 0xC8, "Kpd && ", false},
    { 0xC9, "Kpd | ", false},
    { 0xCA, "Kpd || ", false},
    { 0xCB, "Kpd : ", false},
    { 0xCC, "Kpd # ", false},
    { 0xCD, "Kpd Space ", false},
    { 0xCE, "Kpd @ ", false},
    { 0xCF, "Kpd ! ", false},
    { 0xD0, "Kpd Memory Store ", false},
    { 0xD1, "Kpd Memory Recall ", false},
    { 0xD2, "Kpd Memory Clear ", false},
    { 0xD3, "Kpd Memory Add ", false},
    { 0xD4, "Kpd Memory Subtract ", false},
    { 0xD5, "Kpd Memory Multiply ", false},
    { 0xD6, "Kpd Memory Divide ", false},
    { 0xD7, "Kpd +/- ", false},
    { 0xD8, "Kpd Clear ", false},
    { 0xD9, "Kpd Clear Entry ", false},
    { 0xDA, "Kpd Binary ", false},
    { 0xDB, "Kpd Octal ", false},
    { 0xDC, "Kpd Decimal ", false},
    { 0xDD, "Kpd Hexadecimal ", false},
    
    { 0xE0, "Kb LeftControl", true},
    { 0xE1, "Kb LeftShift", true},
    { 0xE2, "Kb LeftAlt", true},
    { 0xE3, "Kb Left GUI", true},
    { 0xE4, "Kb RightControl", true},
    { 0xE5, "Kb RightShift", true},
    { 0xE6, "Kb RightAlt", true},
    { 0xE7, "Kb Right GUI", true},
};

HIDKeyboard_t * NewHIDKeyboard( void )
{
    HIDKeyboard_t * pKeyboard = malloc( sizeof( HIDKeyboard_t ) );
    memset( pKeyboard, 0, sizeof( HIDKeyboard_t ) );
    int index;
    int count = sizeof( s_HIDKeyInfoConst ) / sizeof( s_HIDKeyInfoConst[0] );
    for ( index = 0; index < count; index++ ) {
        const HIDKeyInfoConst_t *pConst = &s_HIDKeyInfoConst[ index ];
        HIDKeyInfo_t * pKeyInfo = &pKeyboard->keyInfoMap[ pConst->OrgCode ];
        pKeyInfo->pConst = pConst;
        
    }
    return pKeyboard;
}

void HID_PressKey( HIDKeyboard_t * pKeyboard, uint8_t code ) {
    HIDKeyInfo_t * pKeyInfo = &pKeyboard->keyInfoMap[ code ];
    pKeyInfo->Pressed = true;
}

void HID_ReleaseKey( HIDKeyboard_t * pKeyboard, uint8_t code ) {
    HIDKeyInfo_t * pKeyInfo = &pKeyboard->keyInfoMap[ code ];
    pKeyInfo->Pressed = false;
}

void HID_ReleaseAllKeys( HIDKeyboard_t * pKeyboard ) {
    int index;
    for ( index = 0; index < HID_KEY_NUM; index++ ) {
        HIDKeyInfo_t * pKeyInfo = &pKeyboard->keyInfoMap[ index ];
        pKeyInfo->Pressed = false;
    }
}

const HIDKeyInfo_t * HID_GetKeyInfo( HIDKeyboard_t * pKeyboard, uint8_t code ) {
    return &pKeyboard->keyInfoMap[code];
}

void HID_SetConvKeyInfo(
    HIDKeyboard_t * pKeyboard, uint8_t code, const ConvKeyInfo_t * pList, int len )
{
    HIDKeyInfo_t * pKeyInfo = &pKeyboard->keyInfoMap[code];
    pKeyInfo->convKeyInfoLen = len;

    ConvKeyInfo_t * pConvInfo = malloc( sizeof( ConvKeyInfo_t ) * len );
    pKeyInfo->convKeyInfoList = pConvInfo;
    memcpy( pConvInfo, pList, sizeof( ConvKeyInfo_t ) * len );
}

void HID_ClearHIDKeyboard( HIDKeyboard_t * pKeyboard ) {
    int index;
    for ( index = 0; index < HID_KEY_NUM; index++ ) {
        HIDKeyInfo_t * pKeyInfo = &pKeyboard->keyInfoMap[ index ];
        if ( pKeyInfo->convKeyInfoLen > 0 ) {
            free( (void *)pKeyInfo->convKeyInfoList );
            pKeyInfo->convKeyInfoLen = 0;
            pKeyInfo->convKeyInfoList = NULL;
        }
    }
}

void HID_dumpConvKeyInfo( HIDKeyboard_t * pKeyboard ) {
    int index;
    for ( index = 0; index < HID_KEY_NUM; index++ ) {
        const HIDKeyInfo_t * pKeyInfo = HID_GetKeyInfo( pKeyboard, index );
        if ( pKeyInfo->convKeyInfoLen > 0 ) {
            int subIndex;
            for ( subIndex = 0; subIndex < pKeyInfo->convKeyInfoLen; subIndex++ ) {
                const ConvKeyInfo_t * pInfo = &pKeyInfo->convKeyInfoList[ subIndex ];
                printf( "0x%02X: -> Mask:0x%02X, Result:0x%02X, Code:0x%02X, Xor:0x%02X\n",
                        index, pInfo->CondModifierMask,
                        pInfo->CondModifierResult,
                        pInfo->Code, pInfo->ModifierXor );
            }
        }
    }
}


/* func (keyboard *HIDKeyboard) AddConvKey(code byte, convKey *ConvKeyInfo) { */
/* 	keyInfo := keyboard.GetKeyInfo(code) */
/* 	keyInfo.convKeyInfoList = append(keyInfo.convKeyInfoList, convKey) */
/* } */

void HID_SetupHidPackat( HIDKeyboard_t * pKeyboard, HIDReport_t * pReport )
{
    uint8_t orgModifierFlag = 0;
    // 一旦 data をクリアする
    int index;
    for ( index = 0; index < HID_REPORT_LEN; index++ ) {
        pKeyboard->data[index] = 0;
    }
    // modifier をセットする
    int repoIndex = 2;
    for ( index = 0; index < HID_KEY_NUM; index++ ) {
        HIDKeyInfo_t * pKeyInfo = &pKeyboard->keyInfoMap[ index ];
        if ( pKeyInfo->Pressed && pKeyInfo->pConst->IsModifier ) {
            orgModifierFlag |= GetModifierBit( pKeyInfo );
        }
    }
    // キーの置き換え等を処理する
    uint8_t modifierFlag = orgModifierFlag;
    for ( index = 0; index < HID_KEY_NUM; index++ ) {
        HIDKeyInfo_t * pKeyInfo = &pKeyboard->keyInfoMap[ index ];
        if ( pKeyInfo->Pressed ) {
            uint8_t code = HID_process( pKeyInfo, &modifierFlag );
            if ( code > 0 ) {
                pKeyboard->data[ repoIndex ] = code;
                repoIndex++;
                if ( repoIndex >= HID_REPORT_LEN ) {
                    break;
                }
            }
        }
    }
    pKeyboard->data[0] = modifierFlag;
    memcpy( pReport->codes, pKeyboard->data, sizeof( pKeyboard->data ) );
}

#include <stdlib.h>
#include <Config.h>
#include <stdio.h>
#include <string.h>

#define VERSION 1

typedef struct {
    uint32_t ver;
    // HID コードの入れ替え数
    uint32_t switchKeyNum;
    // ConvKeyInfo_t の定義数
    uint32_t convKeyNum;
    // データ本体への offset。 この構造体の先頭からの offset。
    uint32_t offset;
} Conf_header_t;

bool ReadRemap( const void * pBuf, int size,
                Code2HidCode_t * pConv, HIDKeyboard_t * pKeyboard )
{
    const uint8_t * pRaw = pBuf;
    const Conf_header_t * pHeader = (Conf_header_t*)pRaw;
    pRaw += pHeader->offset;
    if ( pHeader->ver != VERSION ) {
        return false;
    }
    if ( pHeader->switchKeyNum > REMAP_CODE_LEN ) {
        return false;
    }

    InitHidRemap( pConv );
    int index;
    for ( index = 0; index < pHeader->switchKeyNum; index++ ) {
        uint8_t oldCode = pRaw[0];
        uint8_t newCode = pRaw[1];
        pRaw += 2;
        SetHIDRemap( pConv, oldCode, newCode );
    }

    HID_ClearHIDKeyboard( pKeyboard );
    for ( index = 0; index < pHeader->convKeyNum; index++ ) {
        uint8_t code = pRaw[ 0 ];
        uint8_t num = pRaw[ 1 ];
        pRaw += 2;
        
        ConvKeyInfo_t * pList = malloc( sizeof( ConvKeyInfo_t ) * num );
        if ( pList == NULL ) {
            return false;
        }
        int subIndex;
        for ( subIndex = 0; subIndex < num; subIndex++ ) {
            ConvKeyInfo_t * pInfo = &pList[ subIndex ];
            pInfo->CondModifierMask = pRaw[ 0 ];
            pInfo->CondModifierResult = pRaw[ 1 ];
            pInfo->Code = pRaw[ 2 ];
            pInfo->ModifierXor = pRaw[ 3 ];
            pRaw += 4;
        }
        HID_SetConvKeyInfo( pKeyboard, code, pList, num );
        free( pList );
    }

    return true;
}

int WriteRemap( void * pBuf, int size,
                const Code2HidCode_t * pConv, const HIDKeyboard_t * pKeyboard )
{
    uint8_t * pRaw = pBuf;
    Conf_header_t * pHeader = (Conf_header_t*)pRaw;
    memset( pHeader, 0, sizeof( *pHeader ) );
    pHeader->ver = VERSION;
    pHeader->offset = sizeof( Conf_header_t );
    pRaw += pHeader->offset;
    int index;
    for ( index = 0; index < REMAP_CODE_LEN; index++ ) {
        if ( pConv->remapHIDCode[ index ] != index ) {
            pHeader->switchKeyNum++;
            
            pRaw[ 0 ] = index;
            pRaw[ 1 ] = pConv->remapHIDCode[ index ];
            pRaw += 2;
        }
    }
    for ( index = 0; index < HID_KEY_NUM; index++ ) {
        const HIDKeyInfo_t * pInfo = &pKeyboard->keyInfoMap[ index ];

        if ( pInfo->convKeyInfoLen > 0 ) {
            pHeader->convKeyNum++;

            pRaw[ 0 ] = index;
            pRaw[ 1 ] = pInfo->convKeyInfoLen;
            pRaw += 2;

            int subIndex;
            for ( subIndex = 0; subIndex < pInfo->convKeyInfoLen; subIndex++ ) {
                const ConvKeyInfo_t * pConvInfo = &pInfo->convKeyInfoList[ subIndex ];
                pRaw[ 0 ] = pConvInfo->CondModifierMask;
                pRaw[ 1 ] = pConvInfo->CondModifierResult;
                pRaw[ 2 ] = pConvInfo->Code;
                pRaw[ 3 ] = pConvInfo->ModifierXor;
                pRaw += 4;
            }
        }
    }
    int len = pRaw - (uint8_t *)pBuf;
    if ( len >= size ) {
        printf( "%s: over size -- %d", __func__, len );
        abort();
    }

    return len;
}

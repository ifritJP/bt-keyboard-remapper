#include <stdio.h>
#include "convMap.h"
#include "esp_log.h"
#include <string.h>
#include "my_keyboard.h"
#include "mutex.h"
#include "Config.h"
#include "my_console.h"
#include "btstack.h"
#include <mbedtls/base64.h>
#include <ctype.h>

#define TAG "MY_KBD"

#define RING_LEN 20

static const char * s_demoTextAscii = "hello world ";

typedef struct ReportRing_t {
    HIDReport_t reportBuf[RING_LEN];
    int readPos;
    int writePos;
    int count;
} ReportRing_t;

static ReportRing_t s_reportRing;
static HIDKeyboard_t * s_pKeyboard = NULL;
static Code2HidCode_t s_conv;

static SMutex_t s_mutex;

static const HIDReport_t s_allReleaseReport = {};


void my_kbd_init( void ) {
    s_conv = newCode2HidCode();
    s_pKeyboard = NewHIDKeyboard();

    s_mutex = SMutex_create();

    my_kbd_load();
}

static bool my_kbd_add_reportRaw( const HIDReport_t * pReport ) {
    bool result;
    SMutex_lock( s_mutex );
    if ( s_reportRing.count < RING_LEN ) {
        s_reportRing.count++;
        s_reportRing.reportBuf[ s_reportRing.writePos ] = *pReport;
        s_reportRing.writePos = (s_reportRing.writePos + 1) % RING_LEN;
        result = true;
        if ( RING_LEN - s_reportRing.count < 5 ) {
            ESP_LOGI( TAG, "%s: ring free count -- %d",
                      __func__, RING_LEN - s_reportRing.count );
        }
    } else {
        ESP_LOGI( TAG, "%s: dropped the report.", __func__ );
        result = false;
    }
    SMutex_unlock( s_mutex );
    return result;
}

bool my_kbd_add_key( uint8_t modifier, uint8_t keycode ) {
    HIDReport_t report = { .codes = { modifier, 0, keycode } };
    // 指定 keycode の report 追加
    if ( !my_kbd_add_reportRaw( &report ) ) {
        return false;
    }
    // key release の report 追加
    return my_kbd_add_reportRaw( &s_allReleaseReport );
}

bool my_kbd_add_report( const HIDReport_t * pReport ) {
    bool result;
    if ( !console_get_isEnableDemo() ) {
        result = my_kbd_add_reportRaw( pReport );
    } else {
        if ( memcmp( pReport->codes, s_allReleaseReport.codes,
                     sizeof( pReport->codes ) ) != 0 )
        {
            uint8_t hidCode = 0x2c; // space
            if ( pReport->codes[ 2 ] == 0x28 ) {// enter
                hidCode = 0x28;
            } else {
                static int pos = 0;
                uint8_t oneChar = tolower( s_demoTextAscii[ pos ] );
                pos = (pos + 1) % strlen( s_demoTextAscii );
                if ( oneChar >= 'a' && oneChar <= 'z' ) {
                    hidCode = oneChar - 'a' + 0x4;
                }
            }
            result = my_kbd_add_key( 0, hidCode );
        } else {
            result = true;
        }
    }
    return result;
}


bool my_kbd_get_report( HIDReport_t * pReport ) {
    bool result;
    SMutex_lock( s_mutex );
    if ( s_reportRing.count > 0 ) {
        s_reportRing.count--;
        *pReport = s_reportRing.reportBuf[ s_reportRing.readPos ];
        s_reportRing.readPos = (s_reportRing.readPos + 1) % RING_LEN;
        result = true;
    } else {
        ESP_LOGI( TAG, "%s: no exsit the report.", __func__ );
        result = false;
    }
    SMutex_unlock( s_mutex );
    return result;
}

bool my_kbd_has_report( void ){
    return s_reportRing.count != 0;
}



void my_kbd_conv( const uint8_t * pSrcReport, int length,
                  HIDReport_t * pReport )
{
    HID_ReleaseAllKeys( s_pKeyboard );

    HIDReport_t report = {};
    if ( length > sizeof( report.codes ) ) {
        length = sizeof( report.codes );
    }
    memcpy( report.codes, pSrcReport, length );

    ProcessHIDReport( &s_conv, s_pKeyboard, &report );

    HID_SetupHidPackat( s_pKeyboard, pReport );
}

void my_kbd_dump( void ) {
    printf( "------- remap ------\n" );
    DumpRemap( &s_conv );

    printf( "------- convert ------\n" );
    HID_dumpConvKeyInfo( s_pKeyboard );
}

void my_kbd_setHidRemap( uint8_t oldCode, uint8_t newCode ) {
    SetHIDRemap( &s_conv, oldCode, newCode );
}

void my_kbd_SetConvKeyInfo( uint8_t code, const ConvKeyInfo_t * pInfo ) {
    HID_SetConvKeyInfo( s_pKeyboard, code, pInfo, 1 );
}

void my_kbd_save( void )
{
    int size = 1000 * 10;
    void * pBuf = malloc( size );
    int dataSize = WriteRemap( pBuf, size, &s_conv, s_pKeyboard );

    console_save_blob( console_blob_id_remap, pBuf, dataSize );
    printf( "%s: dataSize = %d\n", __func__, dataSize );

    free( pBuf );
}

void my_kbd_load( void )
{
    int size = 1000 * 10;
    void * pBuf = malloc( size );

    int dataSize = console_load_blob( console_blob_id_remap, pBuf, size );
    printf( "%s: dataSize = %d\n", __func__, dataSize );
    if ( dataSize > 0 ) {
        ReadRemap( pBuf, dataSize, &s_conv, s_pKeyboard );
    }

    free( pBuf );
}

void my_kbd_dumpBase64( void ) {
    int size = 1000 * 10;
    void * pBuf = malloc( size );
    int dataSize = WriteRemap( pBuf, size, &s_conv, s_pKeyboard );

    printf( "%d\n", dataSize );
    uint8_t * pRaw = pBuf;
    while ( dataSize > 0 ) {
        uint8_t buf[ 80 + 1 ];
        int inputUnit = sizeof( buf ) / 4 * 3;
        size_t len;
        int accessSize = inputUnit;
        if ( accessSize > dataSize ) {
            accessSize = dataSize;
        }
        mbedtls_base64_encode( buf, sizeof( buf ), &len, pRaw, accessSize );
        printf( "%s", buf );

        dataSize -= accessSize;
        pRaw += accessSize;
    }
    printf( "\n" );

    free( pBuf );
}

void my_kbd_readBase64( void ) {

    int dataSize;
    scanf( "%d", &dataSize );
    printf( "dataSize = %d\n", dataSize );

    int size = 1000 * 10;
    void * pBuf = malloc( size );
    uint8_t * pRaw = pBuf;
    
    int b64size = (dataSize + 2) / 3 * 4;
    while ( b64size > 0 ) {
        uint8_t buf[ 80 ];
        int accessSize = sizeof( buf );
        if ( accessSize > b64size ) {
            accessSize = b64size;
        }
        int readSize = fread( buf, 1, accessSize, stdin );
        if ( readSize != accessSize ) {
            break;
        }
        size_t len;
        mbedtls_base64_decode( pRaw, size, &len, buf, readSize );

        pRaw += len;
        b64size -= readSize;
        printf( "remain = %d\n", b64size );
    }
    if ( b64size == 0 ) {
        bool result = ReadRemap( pBuf, dataSize, &s_conv, s_pKeyboard );
        printf( "read = %s\n", result ? "OK" : "NG" );
    } else {
        printf( "fail\n" );
    }

    free( pBuf );
}


void my_kbd_clear(void )
{
    InitHidRemap( &s_conv );
    HID_ClearHIDKeyboard( s_pKeyboard );
}

#include "my_console.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "cmd_decl.h"

#include "my_bluetooth.h"
#include "hid_key.h"
#include <nvs.h>
#include <esp_log.h>
#include "my_ota.h"
#include "cmd_wifi.h"

#include <mbedtls/aes.h>
#include <mbedtls/sha256.h>
#include <string.h>
#include "stateCtrl.h"
#include "btstack/my_gap_inquiry.h"


#define CONSOLE_MAX_ARGS 20

#define TAG "console"


typedef struct {
    void * arg0;
    void * arg1;
    void * arg2;
    void * arg3;
    void * arg4;
    void * arg5;
    void * arg6;
    void * arg7;
    void * arg8;
    void * arg9;
    void * arg10;
    void * arg11;
    void * arg12;
    void * arg13;
    void * arg14;
    void * arg15;
    void * arg16;
    void * arg17;
    void * arg18;
    void * arg19;
    void * arg20;
} console_args_info_t ;

static int console_test_command(int argc, char **argv);
static int console_bt_command(int argc, char **argv);
static int console_myver_command( int argc, char **argv);
static int console_wifi_command( int argc, char **argv);
static int console_state_command( int argc, char **argv);

#include "console.cmd.c"


const int pArgOffsetList[ CONSOLE_MAX_ARGS + 1 ] = {
    offsetof( console_args_info_t, arg0 ),
    offsetof( console_args_info_t, arg1 ),
    offsetof( console_args_info_t, arg2 ),
    offsetof( console_args_info_t, arg3 ),
    offsetof( console_args_info_t, arg4 ),
    offsetof( console_args_info_t, arg5 ),
    offsetof( console_args_info_t, arg6 ),
    offsetof( console_args_info_t, arg7 ),
    offsetof( console_args_info_t, arg8 ),
    offsetof( console_args_info_t, arg9 ),
    offsetof( console_args_info_t, arg10 ),
    offsetof( console_args_info_t, arg11 ),
    offsetof( console_args_info_t, arg12 ),
    offsetof( console_args_info_t, arg13 ),
    offsetof( console_args_info_t, arg14 ),
    offsetof( console_args_info_t, arg15 ),
    offsetof( console_args_info_t, arg16 ),
    offsetof( console_args_info_t, arg17 ),
    offsetof( console_args_info_t, arg18 ),
    offsetof( console_args_info_t, arg19 ),
    offsetof( console_args_info_t, arg20 ),
};

void console_setup( void ) {
    console_register_command(
        s_commandInfo, sizeof( s_commandInfo ) / sizeof( s_commandInfo[0] ) );
}


void console_register_command( const console_command_t * pList, int len ) {
    int index;
    for ( index = 0; index < len; index++ ) {
        const console_command_t * pCommand = &pList[ index ];
        esp_console_cmd_t command = {
            .command = pCommand->name,
            .help = pCommand->help,
            .func = pCommand->func,
            .argtable = pCommand->pArgStrust
        };
        int argIndex;
        for ( argIndex = 0; argIndex < pCommand->argNum; argIndex++ ) {
            if ( argIndex >= CONSOLE_MAX_ARGS ) {
                ESP_LOGE( TAG, "arg max over -- %s", pCommand->name );
                break;
            }
            int offset = pArgOffsetList[ argIndex ];
            void ** ppArg =
                (void **)((uint8_t *)pCommand->pArgStrust + offset);
            const console_command_arg_t * pArgInfo =
                &pCommand->argInfoList[ argIndex ];
            switch ( pArgInfo->type ) {
            case console_command_arg_type_int:
                *ppArg = arg_intn(
                    pArgInfo->shortopts,
                    pArgInfo->longopts,
                    pArgInfo->datatype,
                    pArgInfo->mincount,
                    pArgInfo->maxcount,
                    pArgInfo->glossary);
                break;
            case console_command_arg_type_str:
                *ppArg = arg_strn(
                    pArgInfo->shortopts,
                    pArgInfo->longopts,
                    pArgInfo->datatype,
                    pArgInfo->mincount,
                    pArgInfo->maxcount,
                    pArgInfo->glossary);
                break;
            case console_command_arg_type_flag:
                *ppArg = arg_litn(
                    pArgInfo->shortopts,
                    pArgInfo->longopts,
                    pArgInfo->mincount,
                    pArgInfo->maxcount,
                    pArgInfo->glossary);
                break;
            default:
                assert( false );
                break;
            }
        }
        int endOffset = pArgOffsetList[ pCommand->argNum ];
        void ** ppArg =
            (void **)((uint8_t *)pCommand->pArgStrust + endOffset);
        *ppArg = arg_end( pCommand->argNum );
        
        esp_console_cmd_register( &command );
    }
}


static uint8_t s_key_256[256/8] = {};
const static uint8_t s_iv[16] = {
    0x38, 0x27, 0x89, 0x28, 0x92, 0x73, 0x87, 0x38,
    0x92, 0x89, 0x23, 0x82, 0x93, 0x78, 0x92, 0x78
};

static void console_crypto(
    bool enc, const void * pKey, const void * pBuf, int len, void * pOutBuf )
{
    mbedtls_aes_context ctx;
    
    mbedtls_aes_init(&ctx);
    if ( enc ) {
        mbedtls_aes_setkey_enc(&ctx, pKey, 256);
    } else {
        mbedtls_aes_setkey_dec(&ctx, pKey, 256);
    }

    uint8_t nonce[16];
    memcpy( nonce, s_iv, sizeof( nonce ) );
    
    size_t nc_off = 0;
    mbedtls_aes_crypt_ofb(
        &ctx, len, &nc_off, nonce, (uint8_t*)pBuf, (uint8_t*)pOutBuf );

    mbedtls_aes_free(&ctx);
}


static int console_test_command(int argc, char **argv) {

    //  aes
    const uint8_t key_256[256/8];

    const char *plaintext = "abcdefg";
    
    // allocate internal memory
    const unsigned SZ = strlen( plaintext ) + 1;
    
    uint8_t *chipertext = malloc(SZ);
    uint8_t *decryptedtext = malloc(SZ);

    assert( chipertext );
    assert( decryptedtext );


    console_crypto( true, key_256, plaintext, SZ, chipertext );
    console_crypto( false, key_256, chipertext, SZ, decryptedtext );

    log_info_hexdump( chipertext, SZ );
    printf( "%s\n", decryptedtext );

    free(chipertext);
    free(decryptedtext);


    // sha256
    uint8_t buf[ 32 ];
    mbedtls_sha256( (uint8_t *)plaintext, SZ, buf, 0 );
    log_info_hexdump( buf, sizeof( buf ) );
    
    
    return 0;
}

static int console_bt_command(int argc, char **argv) {
    CONSOLE_PARSE( argc, argv, &s_console_arg_bt_dev );

    if ( s_console_arg_bt_dev.pConnectToDevice->count > 0 ) {
        bd_addr_t addr;
        if ( bt_kb_getAddr(
                 *s_console_arg_bt_dev.pConnectToDevice->ival, addr ) ) {
            bt_kb_host_connect_to_device( addr );
        }
    }
    if ( s_console_arg_bt_dev.pConnectDevice->count > 0 ) {
        bd_addr_t addr;
        if ( bt_kb_getAddr(
                 *s_console_arg_bt_dev.pConnectDevice->ival, addr ) ) {
            bt_kb_device_connect_to_host( addr );
        }
    }
    if ( s_console_arg_bt_dev.pDiscoverable->count > 0 ) {
        bt_kb_set_discoverable( true );
    }
    if ( s_console_arg_bt_dev.pUndiscoverable->count > 0 ) {
        bt_kb_set_discoverable( false );
    }
    if ( s_console_arg_bt_dev.pChannel->count > 0 ) {
        console_dump_hid_channl_info();
    }
    if ( s_console_arg_bt_dev.pSendKey->count > 0 ) {
        bt_kb_set_sendKey( s_console_arg_bt_dev.pSendKey->sval[ 0 ] );
    }
    if ( s_console_arg_bt_dev.pInitHost->count > 0 ) {
        bt_kb_host_setup();
    }
    if ( s_console_arg_bt_dev.pInitDevice->count > 0 ) {
        bt_kb_device_setup();
    }
    if ( s_console_arg_bt_dev.pBle->count > 0 ) {
        key_hid_set_device_mode( hid_device_mode_le );
    }
    if ( s_console_arg_bt_dev.pListConns->count > 0 ) {
        bt_kb_device_list_hci_connection();        
    }
    if ( s_console_arg_bt_dev.pPairedDevices->count ) {
        bt_kb_list_paired_devices();
    }
    if ( s_console_arg_bt_dev.pScan->count > 0 ) {
        if ( strcmp( s_console_arg_bt_dev.pScan->sval[ 0 ], "on" ) == 0 ) {
            my_gap_start_scan();
        } else if ( strcmp( s_console_arg_bt_dev.pScan->sval[ 0 ], "off" ) == 0 ) {
            my_gap_stop_scan();
        } else if ( strcmp( s_console_arg_bt_dev.pScan->sval[ 0 ], "now" ) == 0 ) {
            my_gap_list();
        }
    }
    if ( s_console_arg_bt_dev.pUnpair->count > 0 ) {
        if ( strcmp( s_console_arg_bt_dev.pUnpair->sval[ 0 ], "all" ) == 0 ) {
            gap_delete_all_link_keys();
        } else {
            bd_addr_t addr;
            sscanf_bd_addr( s_console_arg_bt_dev.pUnpair->sval[ 0 ], addr );
            bt_kb_unpair( addr );
        }
    }
    

    return 0;
}

static int console_myver_command( int argc, char **argv)
{
    CONSOLE_PARSE( argc, argv, &s_console_arg_myver );

    printf( "version: 0.00\n" );
    return 0;
}

static int console_get_nvs_str(
    nvs_handle_t nvs, const char * pKey, char * pBuf, int len )
{
    size_t size;
    if ( nvs_get_str(nvs, pKey, NULL, &size ) == ESP_OK) {
        if ( size <= len ) {
            if ( nvs_get_str(nvs, pKey, pBuf, &size ) == ESP_OK ) {
                return size;
            }
        }
    }
    return -1;
}

static int console_get_nvs_blob(
    nvs_handle_t nvs, const char * pKey, void * pBuf, int len )
{
    size_t size;
    if ( nvs_get_blob(nvs, pKey, NULL, &size ) == ESP_OK) {
        if ( size <= len ) {
            if ( nvs_get_blob(nvs, pKey, pBuf, &size ) == ESP_OK ) {
                return size;
            }
        }
    }
    return -1;
}


static bool console_get_nvs_str_sec(
    nvs_handle_t nvs, const char * pKey, char * pBuf, int len )
{
    assert( len <= SEC_TXT_MAX );
    
    char buf[ SEC_TXT_MAX ];
    int actSize = console_get_nvs_blob( nvs, pKey, buf, SEC_TXT_MAX );
    if ( actSize < 0 ) {
        return false;
    }
    console_crypto( false, s_key_256, buf, actSize, pBuf );

    return true;
}

static bool console_set_nvs_str_sec(
    nvs_handle_t nvs, const char * pKey, const char * pTxt )
{
    int len = strlen( pTxt ) + 1;
    assert( len <= SEC_TXT_MAX );

    char buf[ SEC_TXT_MAX ];
    console_crypto( true, s_key_256, pTxt, len, buf );
    
    if ( nvs_set_blob( nvs, pKey, buf, len ) != ESP_OK ) {
        return false;
    }

    return true;
}


bool console_get_wifi_setting( wifi_setting_t * pSetting ) {
    nvs_handle_t nvs;
    esp_err_t err = nvs_open( "ifritJP", NVS_READWRITE, &nvs);
    if ( err != ESP_OK ) {
        return false;
    }

    bool success = true;
    if ( console_get_nvs_blob(
             nvs, "cryptKey", s_key_256, sizeof( s_key_256 ) ) < 0 )
    {
        success = false;
    }
    
    if ( !console_get_nvs_str_sec(
             nvs, "ssid", pSetting->ssid, sizeof( pSetting->ssid ) ) ) {
        success = false;
    }
    if ( !console_get_nvs_str_sec(
             nvs, "pass", pSetting->pass, sizeof( pSetting->pass ) ) ) {
        success = false;
    }
    if ( !console_get_nvs_str_sec(
             nvs, "otaauthb64", pSetting->otaAuthB64, sizeof( pSetting->otaAuthB64 ) ) ) {
        strcpy( pSetting->otaAuthB64, "" );
    }
    
    
    nvs_close(nvs);
    return success;
}

static int console_wifi_command( int argc, char **argv)
{
    CONSOLE_PARSE( argc, argv, &s_console_arg_wifi );

    nvs_handle_t nvs;
    esp_err_t err = nvs_open( "ifritJP", NVS_READWRITE, &nvs);
    ESP_LOGI( TAG, "%d", err );
    if ( s_console_arg_wifi.pCryptKey->count > 0 ) {
        const char * pCryptKey = s_console_arg_wifi.pCryptKey->sval[0];
        mbedtls_sha256( (uint8_t *)pCryptKey, strlen( pCryptKey ), s_key_256, 0 );
        err = nvs_set_blob( nvs, "cryptKey", s_key_256, sizeof( s_key_256 ) );
    }
    ESP_LOGI( TAG, "%d", err );
    if ( s_console_arg_wifi.pSsid->count > 0 ) {
        err = console_set_nvs_str_sec(
            nvs, "ssid", s_console_arg_wifi.pSsid->sval[0] );
    }
    ESP_LOGI( TAG, "%d", err );
    if ( s_console_arg_wifi.pPass->count > 0 ) {
        err = console_set_nvs_str_sec(
            nvs, "pass", s_console_arg_wifi.pPass->sval[0] );
    }
    ESP_LOGI( TAG, "%d", err );
    if ( s_console_arg_wifi.pOtaAuthB64->count > 0 ) {
        err = console_set_nvs_str_sec(
            nvs, "otaauthb64", s_console_arg_wifi.pOtaAuthB64->sval[0] );
    }
    if ( s_console_arg_wifi.pClear->count > 0 ) {
        nvs_erase_key( nvs, "cryptKey" );
        nvs_erase_key( nvs, "ssid" );
        nvs_erase_key( nvs, "pass" );
        nvs_erase_key( nvs, "otaauthb64" );
    }
    if ( s_console_arg_wifi.pDump->count > 0 ) {
        wifi_setting_t setting;
        if ( console_get_wifi_setting( &setting ) ) {
            printf( "OK\n" );
        }
    }
    
    err = nvs_commit(nvs);
    ESP_LOGI( TAG, "%d", err );
    nvs_close(nvs);

    if ( s_console_arg_wifi.pOta->count > 0 ) {
        wifi_setting_t setting;
        if ( console_get_wifi_setting( &setting ) ) {
            wrap_wifi_join( setting.ssid, setting.pass, 2000 );
            start_otad( setting.otaAuthB64 );
        }
    }
    
    return 0;
}

static int console_state_command( int argc, char **argv)
{
    CONSOLE_PARSE( argc, argv, &s_console_arg_state );

    state_dump();
    
    return 0;
}

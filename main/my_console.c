// -*- coding: utf-8; -*-

#include "my_console.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "cmd_decl.h"

#include "my_bluetooth.h"
#include "hid_keyboard_ctrl.h"
#include <nvs.h>
#include <esp_log.h>
#include "my_ota.h"
#include "cmd_wifi.h"

#include <mbedtls/aes.h>
#include <mbedtls/sha256.h>
#include <string.h>
#include "stateCtrl.h"
#include "btstack/my_gap_inquiry.h"
#include <my_keyboard.h>
#include <stdlib.h>
#include <hog_key.h>


#define BT_INFO_VER 1

#define CONSOLE_MAX_ARGS 20

#define TAG "console"

#define BT_NAMESPACE "bt@ifritJP"

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

static int console_get_nvs_blob_ns(
    const char * pNameSpace, const char * pKey, void * pBuf, int len );


static int console_test_command(int argc, char **argv);
static int console_bt_command(int argc, char **argv);
static int console_myver_command( int argc, char **argv);
static int console_wifi_command( int argc, char **argv);
static int console_state_command( int argc, char **argv);
static int console_remap_command( int argc, char **argv);
//static int console_console_command( int argc, char **argv);
static int console_config_command( int argc, char **argv);

#include "my_console.cmd.c"

static my_config_t s_config = {
    .version = 0,
    .mode = my_config_mode_setup,
    .hid_device_mode = hid_device_mode_bt,
    .isEnableDemo = false,
};

typedef struct {
    bt_type_t type;
    bt_roleType_t role;
    bd_addr_t addr;
    bd_addr_type_t addrType;
} console_bt_info_t;
static console_bt_info_t s_console_bt_info_host;
static console_bt_info_t s_console_bt_info_device;

static char s_bd_addr_key_buf[ 16 ];

static bool console_set_nvs_blob_ns(
    const char * pNameSpace, const char * pKey, const void * pBuf, int len );

static void console_save_config( void ) {
    console_set_nvs_blob_ns(
        BT_NAMESPACE, "config", &s_config, sizeof( s_config ) );
}

void console_save_blob(
    console_blob_id_t id, const void * pBuf, int size )
{
    char buf[ 20 ];
    snprintf( buf, sizeof( buf ), "blob%d", id );
    buf[ sizeof( buf ) - 1 ] = '\0';
    console_set_nvs_blob_ns( BT_NAMESPACE, buf, pBuf, size );
}

int console_load_blob( console_blob_id_t id, void * pBuf, int size )
{
    char buf[ 20 ];
    snprintf( buf, sizeof( buf ), "blob%d", id );
    buf[ sizeof( buf ) - 1 ] = '\0';
    return console_get_nvs_blob_ns( BT_NAMESPACE, buf, pBuf, size );
}

static const char * console_bd_addr_2_key( const bd_addr_t addr ) {
    int index;
    char * pBuf = s_bd_addr_key_buf;
    for ( index = 0; index < sizeof( bd_addr_t ); index++ ) {
        sprintf( pBuf, "%02X", (uint8_t)addr[ index ] );
        pBuf += 2;
    }
    *pBuf = '\0';
    return s_bd_addr_key_buf;
}

static console_bt_info_t * console_get_bt_for_role( bt_roleType_t roleType ) {
    if ( roleType == bt_roleType_device ) {
        return &s_console_bt_info_device;
    }
    return &s_console_bt_info_host;
}
static console_bt_info_t * console_get_bt_for_addr( bd_addr_t addr ) {
    if ( memcmp( s_console_bt_info_device.addr, addr, sizeof( bd_addr_t ) ) == 0 ) {
        return &s_console_bt_info_device;
    }
    if ( memcmp( s_console_bt_info_host.addr, addr, sizeof( bd_addr_t ) ) == 0 ) {
        return &s_console_bt_info_host;
    }
    return NULL;
}


static void console_update_btInfo(
    bd_addr_t addr, bd_addr_type_t addrType,
    bt_roleType_t roleType, bt_type_t btType )
{
    my_bt_info_t btInfo;
    if ( !console_get_bt_info( addr, &btInfo ) ) {
        ESP_LOGI( TAG, "CONSOLE_BT_STATE_CONNECTING" );

        console_bt_info_t * pInfo = console_get_bt_for_role( roleType );
        pInfo->type = btType;
        pInfo->role = roleType;
        memcpy( pInfo->addr, addr, sizeof( bd_addr_t ) );
        pInfo->addrType = addrType;
        gap_remote_name_request( pInfo->addr, 0, 0x8000 );
    }
    if ( roleType == bt_roleType_host ) {
        memcpy( s_config.toHostAddr, addr, sizeof( bd_addr_t ) );
        console_save_config();
    }
}

void console_hid_packet_handler_meta_bt(
    uint16_t channel, const uint8_t * packet, bt_roleType_t roleType )
{
    uint8_t subevent = hci_event_hid_meta_get_subevent_code(packet);
    uint8_t status;
    ESP_LOGI( TAG, "%s: %d", __func__, subevent );
    switch ( subevent ) {
    case HID_SUBEVENT_CONNECTION_OPENED:
        ESP_LOGI( TAG, "HID_SUBEVENT_CONNECTION_OPENED" );
        
        status = hid_subevent_connection_opened_get_status(packet);
        if (status == ERROR_CODE_SUCCESS) {
            bd_addr_t addr;
            hid_subevent_connection_opened_get_bd_addr( packet, addr );

            console_update_btInfo(
                addr, BD_ADDR_TYPE_LE_PUBLIC, roleType, bt_type_classic );
        }
        break;
    }
}

static bd_addr_t s_pairingAddrForLe;
static bd_addr_t s_connectAddrForLe;


void console_hid_packet_handler_meta_le(
    uint16_t channel, const uint8_t * packet, bt_roleType_t roleType )
{
    uint8_t subevent = hci_event_hids_meta_get_subevent_code(packet);

    ESP_LOGI( TAG, "%s: %d", __func__, subevent );
    switch ( subevent ) {
    case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
        {
            // HIDS_SUBEVENT_INPUT_REPORT_ENABLE のイベント情報だと、
            // bd_addr_t が RANDOM になっているので
            // SM_EVENT_PAIRING_COMPLETE のアドレスを使用する。
            uint16_t con_handle =
                hids_subevent_input_report_enable_get_con_handle(packet);
            hci_connection_t * connection =
                hci_connection_for_handle(con_handle);
            printf( "addrType = %d\n", connection->address_type );

            memcpy( s_connectAddrForLe, connection->address, sizeof( bd_addr_t ) );
            
            bd_addr_t addr = {};
            if ( memcmp( addr, s_pairingAddrForLe, sizeof( bd_addr_t ) ) != 0 ) {
                console_update_btInfo(
                    s_pairingAddrForLe,
                    connection->address_type, roleType, bt_type_le );
            }
        }
        break;
    }
}

static void packet_handler(
    uint8_t packet_type, uint16_t channel,
    uint8_t * packet, uint16_t packet_size )
{
    
    
    switch (packet_type){
    case HCI_EVENT_PACKET:
        //ESP_LOGI( TAG, "event %X", hci_event_packet_get_type(packet) );
        switch (hci_event_packet_get_type(packet)) {
        case SM_EVENT_JUST_WORKS_REQUEST:
            ESP_LOGI( TAG, "Just Works requested" );
            {
                hci_con_handle_t handle =
                    sm_event_just_works_request_get_handle(packet);
                if ( console_get_config_mode() == my_config_mode_setup ) {
                    sm_just_works_confirm( handle );
                } else {
                    sm_bonding_decline( handle );
                }
            }
            break;
        case SM_EVENT_NUMERIC_COMPARISON_REQUEST:
            ESP_LOGI( TAG, "Confirming numeric comparison: %u\n",
                      sm_event_numeric_comparison_request_get_passkey(packet) );
            {
                hci_con_handle_t handle =
                    sm_event_numeric_comparison_request_get_handle(packet);
                if ( console_get_config_mode() == my_config_mode_setup ) {
                    sm_numeric_comparison_confirm( handle );
                } else {
                    sm_bonding_decline( handle );
                }
            }
            break;
        case SM_EVENT_PASSKEY_DISPLAY_NUMBER:
            {
                uint32_t passkey = sm_event_passkey_display_number_get_passkey(packet);
                ESP_LOGI( TAG, "Display Passkey: %u\n", passkey );
                {
                    hci_con_handle_t handle =
                        sm_event_passkey_display_number_get_handle( packet );
                    if ( console_get_config_mode() == my_config_mode_setup ) {
                        // sm_passkey_input( handle, passkey );
                    } else {
                        sm_bonding_decline( handle );
                    }
                }
            }
            break;
        case HCI_EVENT_LE_META:
            /* ESP_LOGI( TAG, "le_meta_subevent %X", */
            /*           hci_event_le_meta_get_subevent_code(packet) ); */

            /* switch (hci_event_le_meta_get_subevent_code(packet)) { */
            /* case HCI_SUBEVENT_LE_CONNECTION_COMPLETE: */
            /*     { */
            /*         bd_addr_t addr; */
            /*         hci_subevent_le_connection_complete_get_peer_address( */
            /*             packet, addr ); */
            /*         uint8_t address_type = */
            /*             hci_subevent_le_connection_complete_get_peer_address_type( packet ); */
            /*         hci_con_handle_t con = */
            /*             hci_subevent_le_connection_complete_get_connection_handle( packet ); */
            /*         ESP_LOGI( TAG, "connection_complete: %s %d %d", */
            /*                   bd_addr_to_str( addr ), address_type, con ); */
            /*     } */
            /*     break; */
            /* } */
            break;
        case SM_EVENT_PAIRING_STARTED:
            memset( s_pairingAddrForLe, 0, sizeof( bd_addr_t ) );
            break;
        case SM_EVENT_PAIRING_COMPLETE:
            {
                uint8_t status = sm_event_pairing_complete_get_status( packet );
                if ( status == ERROR_CODE_SUCCESS ) {
                    sm_event_pairing_complete_get_address( packet, s_pairingAddrForLe );
                }
                ESP_LOGI( TAG, "COMPLETE %s, %d",
                          bd_addr_to_str( s_pairingAddrForLe ), status );
            }
            break;
        case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
            {
                bd_addr_t addr;
                hci_event_remote_name_request_complete_get_bd_addr( packet, addr );

                
                console_bt_info_t * pInfo = console_get_bt_for_addr( addr );
                if ( pInfo != NULL ) {
                    memset( pInfo->addr, 0, sizeof( bd_addr_t ) );
                    
                    my_bt_info_t btInfo;
                    btInfo.version = BT_INFO_VER;
                    btInfo.type = pInfo->type;
                    btInfo.role = pInfo->role;
                    btInfo.pairAddrType = pInfo->addrType;
                    memcpy( btInfo.addr, addr, sizeof( addr ) );

                    if ( pInfo->role == bt_roleType_host ) {
                        if ( console_get_hid_device_mode() == hid_device_mode_bt ) {
                            memcpy( btInfo.pairAddr, addr, sizeof( addr ) );
                        } else {
                            memcpy( btInfo.pairAddr,
                                    s_connectAddrForLe, sizeof( s_connectAddrForLe ) );
                        }
                    } else {
                        memcpy( btInfo.pairAddr, addr, sizeof( addr ) );
                    }

                    const char * pName =
                        hci_event_remote_name_request_complete_get_remote_name( packet );
                    strncpy( btInfo.name, pName, sizeof( btInfo.name ) - 1 );
                    btInfo.name[ sizeof( btInfo.name ) - 1 ] = '\0';

                    ESP_LOGI( TAG, "REQ_COMPLETE %s %s",
                              bd_addr_to_str( addr ), btInfo.name );

                    console_set_nvs_blob_ns(
                        BT_NAMESPACE, console_bd_addr_2_key( btInfo.addr ),
                        &btInfo, sizeof( btInfo ) );
                }
            }
            break;
        }
        break;
    }
}



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

void console_loadConfig( void ) {
    console_get_nvs_blob_ns(
        BT_NAMESPACE, "config", &s_config, sizeof( s_config ) );
}

void console_setup( void ) {
    console_register_command(
        s_commandInfo, sizeof( s_commandInfo ) / sizeof( s_commandInfo[0] ) );

    static btstack_packet_callback_registration_t event_callback_registration;

    event_callback_registration.callback = packet_handler;
    hci_add_event_handler(&event_callback_registration);
    sm_add_event_handler(&event_callback_registration);
    
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

static bool console_set_nvs_blob_ns(
    const char * pNameSpace, const char * pKey, const void * pBuf, int len )
{
    nvs_handle_t nvs;
    esp_err_t err = nvs_open( pNameSpace, NVS_READWRITE, &nvs );
    if ( err != ESP_OK ) {
        return false;
    }

    bool result = ( nvs_set_blob( nvs, pKey, pBuf, len ) != ESP_OK );

    nvs_close( nvs );
    
    return result;
}

static int console_get_nvs_blob_ns(
    const char * pNameSpace, const char * pKey, void * pBuf, int len )
{
    nvs_handle_t nvs;
    esp_err_t err = nvs_open( pNameSpace, NVS_READWRITE, &nvs );
    if ( err != ESP_OK ) {
        return -1;
    }
    
    int result = console_get_nvs_blob( nvs, pKey, pBuf, len );

    nvs_close( nvs );
    
    return result;
}

bool console_get_bt_info( const bd_addr_t addr, my_bt_info_t * pInfo )
{
    if ( console_get_nvs_blob_ns(
             BT_NAMESPACE, console_bd_addr_2_key( addr ),
             pInfo, sizeof( *pInfo ) ) != sizeof( *pInfo ) ) {
        return false;
    }
    if ( pInfo->version != BT_INFO_VER ) {
        return false;
    }
    return true;
}

static bool console_del_nvs_blob_ns( const char * pNameSpace, const char * pKey )
{
    nvs_handle_t nvs;
    esp_err_t err = nvs_open( pNameSpace, NVS_READWRITE, &nvs );
    if ( err != ESP_OK ) {
        return false;
    }

    bool result = ( nvs_erase_key( nvs, pKey ) != ESP_OK );

    nvs_close( nvs );
    
    return result;
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
        bd_addr_type_t addrType;
        if ( bt_kb_getPairAddr(
                 *s_console_arg_bt_dev.pConnectToDevice->ival, addr, &addrType ) )
        {
            bt_kb_host_connect_to_device( addr );
        }
    }
    if ( s_console_arg_bt_dev.pConnectDevice->count > 0 ) {
        bd_addr_t addr;
        bd_addr_type_t addrType;
        if ( bt_kb_getPairAddr(
                 *s_console_arg_bt_dev.pConnectDevice->ival, addr, &addrType ) )
        {
            bt_kb_device_connect_to_host( addr, addrType );
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

            while ( true ) {
                bool processed = false;
                nvs_iterator_t itr =
                    nvs_entry_find( "nvs", BT_NAMESPACE, NVS_TYPE_BLOB );
                for (; itr != NULL; itr = nvs_entry_next( itr ) ) {
                    nvs_entry_info_t entry;
                    nvs_entry_info( itr, &entry );
                    if ( strcmp( entry.key, "config" ) != 0 ) {
                        processed = true;
                        console_del_nvs_blob_ns( BT_NAMESPACE, entry.key );
                        break;
                    }
                }
                nvs_release_iterator( itr );
                if ( !processed ) {
                    break;
                }
            }

            int index = 0;
            for ( index = 0; index < le_device_db_max_count(); index++ ) {
                int addr_type;
                bd_addr_t addr;
                le_device_db_info( index, &addr_type, addr, NULL );
                if ( addr_type != BD_ADDR_TYPE_UNKNOWN ) {
                    le_device_db_remove( index );
#ifdef ENABLE_LE_PRIVACY_ADDRESS_RESOLUTION
                    hci_remove_le_device_db_entry_from_resolving_list(i);
#endif
                }
            }
            {
                nvs_handle_t nvs;
                if ( nvs_open( "BTstack", NVS_READWRITE, &nvs ) != ESP_OK ) {
                    ESP_LOGE( TAG, "failed the nvs_open()" );
                } else {
                    nvs_erase_all( nvs );
                }
            }
            printf( "please restart.\n" );
        } else {
            bd_addr_t addr;
            sscanf_bd_addr( s_console_arg_bt_dev.pUnpair->sval[ 0 ], addr );
            bt_kb_unpair( addr );
            console_del_nvs_blob_ns( BT_NAMESPACE, console_bd_addr_2_key( addr ) );
        }
    }

    if ( s_console_arg_bt_dev.pPasskey->count > 0 ) {
        const char * pTxt = s_console_arg_bt_dev.pPasskey->sval[0];
        hog_send_passkey( strtol( pTxt, NULL, 10 ) );
    }
    

    return 0;
}

static int console_myver_command( int argc, char **argv)
{
    CONSOLE_PARSE( argc, argv, &s_console_arg_myver );

    printf( "version: 0.00\n" );
    return 0;
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

static int parseStr( char * pTxt, char delimit,
                     const char ** pTokenBuf, int bufLen )
{
    int count = 0;
    while ( true ) {
        if ( bufLen <= count ) {
            return count;
        }
        pTokenBuf[ count ] = pTxt;
        count++;
        char * pFind = strchr( pTxt, delimit );
        if ( pFind == NULL ) {
            return count;
        }
        *pFind = '\0';
        pTxt = pFind + 1;
    }
}

static int console_remap_command( int argc, char **argv)
{
    CONSOLE_PARSE( argc, argv, &s_console_arg_remap );

    if ( s_console_arg_remap.pUpload->count > 0 ) {
    }
    if ( s_console_arg_remap.pDump->count > 0 ) {
        my_kbd_dump();
    }
    if ( s_console_arg_remap.pKey->count > 0 ) {
        const char * pParams[ 2 ];
        if ( parseStr(
                 (char *)s_console_arg_remap.pKey->sval[0], ',', pParams, 2 ) == 2 ) {
            my_kbd_setHidRemap( strtol( pParams[0], NULL, 10 ),
                                strtol( pParams[1], NULL, 10 ) );
        }
    }
    if ( s_console_arg_remap.pConv->count > 0 ) {
        const char * pParams[ 5 ];
        if ( parseStr(
                 (char *)s_console_arg_remap.pConv->sval[0], ',', pParams, 5 ) == 5 ) {
            ConvKeyInfo_t conv = {
                strtol( pParams[1], NULL, 10 ),
                strtol( pParams[2], NULL, 10 ),
                strtol( pParams[3], NULL, 10 ),
                strtol( pParams[4], NULL, 10 ),
            };
            my_kbd_SetConvKeyInfo( strtol( pParams[0], NULL, 10 ), &conv );
        }
    }
    if ( s_console_arg_remap.pSave->count > 0 ) {
        my_kbd_save();
    }
    if ( s_console_arg_remap.pLoad->count > 0 ) {
        my_kbd_load();
    }
    if ( s_console_arg_remap.pClear->count > 0 ) {
        my_kbd_clear();
    }
    if ( s_console_arg_remap.pDump64->count > 0 ) {
        my_kbd_dumpBase64();
    }
    if ( s_console_arg_remap.pLoad64->count > 0 ) {
        my_kbd_readBase64();
    }

    return 0;
}

static int console_state_command( int argc, char **argv)
{
    CONSOLE_PARSE( argc, argv, &s_console_arg_state );

    state_dump();

    return 0;
}

static int console_config_command( int argc, char **argv)
{
    CONSOLE_PARSE( argc, argv, &s_console_arg_config );

    const my_config_t backConfig = s_config;
    
    
    if ( s_console_arg_config.pMode->count > 0 ) {
        const char * pMode = s_console_arg_config.pMode->sval[0];
        if ( strcmp( pMode, "normal" ) == 0 ) {
            s_config.mode = my_config_mode_normal;
        } else if ( strcmp( pMode, "setup" ) == 0 ) {
            s_config.mode = my_config_mode_setup;
        }
    }
    if ( s_console_arg_config.pDevMode->count > 0 ) {
        const char * pMode = s_console_arg_config.pDevMode->sval[0];
        if ( strcmp( pMode, "bt" ) == 0 ) {
            s_config.hid_device_mode = hid_device_mode_bt;
        } else if ( strcmp( pMode, "le" ) == 0 ) {
            s_config.hid_device_mode = hid_device_mode_le;
        }
    }
    if ( s_console_arg_config.pDemo->count > 0 ) {
        s_config.isEnableDemo = s_console_arg_config.pDemo->ival[ 0 ] != 0;
    }

    if ( memcmp( &backConfig, &s_config, sizeof( s_config ) ) != 0 ) {
        console_save_config();

        printf( "need to restrart appling the config.\n" );
    }

    printf( " mode: %s\n",
            s_config.mode == my_config_mode_setup ? "setup" : "normal" );
    printf( " toHostAddr: %s\n", bd_addr_to_str( s_config.toHostAddr ) );
    printf( " hidDeviceMode: %s\n",
            s_config.hid_device_mode == hid_device_mode_bt ? "bt" : "le" );
    printf( " demoMode: %d\n", s_config.isEnableDemo );
    
    return 0;
}

my_config_t * console_get_config( void ) {
    return &s_config;
}

my_config_mode_t console_get_config_mode( void )
{
    return s_config.mode;
}

hid_device_mode_t console_get_hid_device_mode( void ) {
    return s_config.hid_device_mode;
}

bool console_get_isEnableDemo( void ) {
    return s_config.isEnableDemo;
}

#include "hid_keyboard_ctrl.h"
#include "my_keyboard.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <esp_log.h>

#include "btstack.h"
#include "my_console.h"
#include "my_bluetooth.h"
#include "hog_key.h"
#include "console.h"
#include "stateCtrl.h"
#include "hid_reportDesc.h"

#define TAG "HID_CTRL"

static hid_protocol_mode_t s_hid_host_report_mode =
    HID_PROTOCOL_MODE_REPORT_WITH_FALLBACK_TO_BOOT;
static bool s_hid_host_descriptor_available = false;


static const char s_hid_device_name[] = "BT Keyboard Remapper 00:00:00:00:00:00";

static uint8_t s_hid_service_buffer[300];
static uint8_t s_device_id_sdp_service_buffer[100];
static btstack_packet_callback_registration_t s_hci_event_callback_registration;

typedef enum {
    HID_STATE_BOOTING,
    HID_STATE_NOT_CONNECTED,
    HID_STATE_CONNECTED
} hid_state_t;

typedef struct {
    uint16_t channel;
    hid_state_t state;
    uint16_t cid;
    bd_addr_t addr;
} hid_info_t;

static hid_info_t s_hid_info_in = { 0, HID_STATE_BOOTING, 0, {} };
static hid_info_t s_hid_info_out = { 0, HID_STATE_BOOTING, 0, {} };

typedef enum {
    CHANNEL_STATE_FREE,
    CHANNEL_STATE_RESERVE,
    CHANNEL_STATE_CONNECTED,
} channel_state_t;

typedef enum {
    hid_channel_type_none,
    hid_channel_type_device,
    hid_channel_type_host
} hid_channel_type_t;

typedef struct {
    channel_state_t state;
    uint16_t number;
    hid_channel_type_t type;
    uint16_t age;
} hid_channel_info_t;
#define MAX_HID_CHANNEL_INFO 10
static hid_channel_info_t s_hid_channel_infoList[ MAX_HID_CHANNEL_INFO ] = {};


void console_dump_hid_channl_info( void ) {
    int index;

    printf( "%2s: %2s %5s %5s %s\n",
            "", "ch", "age", "kind", "state" );
    
    for ( index = 0; index < MAX_HID_CHANNEL_INFO; index++ ) {
        hid_channel_info_t * pInfo = &s_hid_channel_infoList[ index ];
        if ( pInfo->state != CHANNEL_STATE_FREE ) {
            const char * pKind = "";
            switch ( pInfo->type ) {
            case hid_channel_type_device:
                pKind = "dev";
                break;
            case hid_channel_type_host:
                pKind = "host";
                break;
            default:
                pKind = "none";
                break;
            }
            printf( "%2d: %2d %5X %5s %s\n",
                    index, pInfo->number, pInfo->age, pKind,
                    pInfo->state == CHANNEL_STATE_RESERVE ?
                    "reserve" : "connect" );
        }
    }

    printf( "\n" );
    printf( "s_hid_info_in:\n" );
    printf( "  channel = %d, cid = %d, state = %d, addr = %s\n",
            s_hid_info_in.channel, s_hid_info_in.cid,
            s_hid_info_in.state, bd_addr_to_str( s_hid_info_in.addr ) );
    printf( "s_hid_info_out:\n" );
    printf( "  channel = %d, cid = %d, state = %d, addr = %s\n",
            s_hid_info_out.channel, s_hid_info_out.cid,
            s_hid_info_out.state, bd_addr_to_str( s_hid_info_out.addr ) );
}

uint8_t key_hid_host_connect( bd_addr_t addr ) {
    return hid_host_connect( addr, s_hid_host_report_mode, &s_hid_info_in.cid);
}

static bd_addr_t s_connecting_device_target_addr;
void key_hid_clear_device_target_addr( void ) {
    memset( s_connecting_device_target_addr, 0, sizeof( bd_addr_t ) );
}

void key_hid_device_connect( bd_addr_t addr ) {
    memcpy( s_connecting_device_target_addr, addr, sizeof( bd_addr_t ) );
    hid_device_connect( addr, &s_hid_info_in.cid );
}

static bool IsInputDeviceConnected( void ) {
    if ( console_get_hid_device_mode() == hid_device_mode_bt ) {
        return s_hid_info_out.cid != 0;
    } else {
        return hog_isConnected();
    }
}

static void processHidMetaDevice(
    hid_channel_info_t * pChannelInfo, uint16_t channel, uint8_t * packet )
{
    console_hid_packet_handler_meta_bt( channel, packet, bt_roleType_host );
    
    uint16_t cid = hid_subevent_connection_opened_get_hid_cid(packet);
    uint8_t subevent = hci_event_hid_meta_get_subevent_code(packet);
    switch ( subevent ){
    case HID_SUBEVENT_CONNECTION_OPENED:
        {
            uint8_t status = hid_subevent_connection_opened_get_status(packet);

            {
                bd_addr_t addr;
                hid_subevent_connection_opened_get_bd_addr( packet, addr );
                uint8_t incoming =
                    hid_subevent_connection_opened_get_incoming( packet );
                hci_con_handle_t handle =
                    hid_subevent_connection_opened_get_con_handle( packet );
                printf( "%s: %d, %s, %d \n", __func__, incoming,
                        bd_addr_to_str( addr), handle );

                if ( memcmp( addr, s_connecting_device_target_addr,
                             sizeof( bd_addr_t ) != 0 ) )
                {
                    hid_device_disconnect( cid );
                    return;
                }
            }

            hid_info_t * pInfo = &s_hid_info_out;
                            
            if (status != ERROR_CODE_SUCCESS) {
                // outgoing connection failed
                printf("Connection failed, status 0x%x\n", status);
                pInfo->state = HID_STATE_NOT_CONNECTED;
                pInfo->cid = 0;
                pInfo->channel = 0;
                return;
            }

            state_connectedToHost( true );
            
            pInfo->state = HID_STATE_CONNECTED;
            hid_subevent_connection_opened_get_bd_addr( packet, pInfo->addr );
            
            pInfo->cid = cid;
            pInfo->channel = channel;

            pChannelInfo->state = CHANNEL_STATE_CONNECTED;
        }
        break;
    case HID_SUBEVENT_CONNECTION_CLOSED:
        printf("HID Disconnected\n");
                            
        hid_info_t * pInfo = &s_hid_info_out;
        memset( pInfo->addr, 0, sizeof( bd_addr_t ) );
        pInfo->state = HID_STATE_NOT_CONNECTED;
        pInfo->cid = 0;
        pChannelInfo->state = CHANNEL_STATE_RESERVE;

        state_connectedToHost( false );
        
        break;
    case HID_SUBEVENT_CAN_SEND_NOW:
        if ( console_get_hid_device_mode() == hid_device_mode_bt ) {
            HIDReport_t report;
            if ( my_kbd_get_report( &report ) ) {
                uint8_t hidReport[ 10 ] = { 0xa1, 0x01, };
                memcpy( hidReport + 2, report.codes, 8 );
                if ( console_get_config_mode() == my_config_mode_setup ) {
                    log_info_hexdump( hidReport, sizeof( hidReport ) );
                }
                
                hid_device_send_interrupt_message(
                    s_hid_info_out.cid, hidReport, sizeof(hidReport));
                if ( my_kbd_has_report() ) {
                    hid_device_request_can_send_now_event( s_hid_info_out.cid );
                }
            }
        }
        break;
    default:
        break;
    }
}

static hid_channel_info_t * updateChannelInfo(
    uint16_t channel, hid_channel_type_t type )
{
    int index;

    hid_channel_info_t * pFreeInfo = NULL;
    hid_channel_info_t * pOldestReserveInfo = NULL;


    // s_hid_channel_infoList の中から
    // channel を管理する hid_channel_info_t を見つける。
    // あれば処理終了。
    // なければ以下をセット。
    // - pFreeInfo に、未使用の hid_channel_info_t をセット。
    // - pOldestReserveInfo に、一番古く確保された hid_channel_info_t をセット
    // 
    for ( index = 0; index < MAX_HID_CHANNEL_INFO; index++ ) {
        hid_channel_info_t * pInfo = &s_hid_channel_infoList[ index ];
        if ( pInfo->state == CHANNEL_STATE_FREE ) {
            if ( pFreeInfo == NULL ) {
                pFreeInfo = pInfo;
            }
        } else {
            if ( pInfo->number == channel ) {
                // channel を管理するものが見つかった場合は終了
                return pInfo;
            }
            // pOldestReserveInfo のセット
            if ( pInfo->state == CHANNEL_STATE_RESERVE ) {
                if ( pOldestReserveInfo == NULL ) {
                    pOldestReserveInfo = pInfo;
                } else {
                    if ( pOldestReserveInfo->age < pInfo->age ) {
                        pOldestReserveInfo = pInfo;
                    }
                }
            }
        }
    }

    // reserved の age をインクリメント
    for ( index = 0; index < MAX_HID_CHANNEL_INFO; index++ ) {
        hid_channel_info_t * pInfo = &s_hid_channel_infoList[ index ];
        if ( pInfo->state == CHANNEL_STATE_RESERVE ) {
            pInfo->age++;
        }
    }

    if ( pFreeInfo == NULL ) {
        assert( pOldestReserveInfo != NULL );
        pFreeInfo = pOldestReserveInfo;
    }
    pFreeInfo->state = CHANNEL_STATE_RESERVE;
    pFreeInfo->number = channel;
    pFreeInfo->type = type;
    pFreeInfo->age = 0;

    printf("%s: reserve hid_channel_info_t -- %d, %d\n",
           __func__, index, type );

    return pFreeInfo;
}

static void packet_handler_device(
    uint8_t packet_type, uint16_t channel,
    uint8_t * packet, uint16_t packet_size)
{
    UNUSED(channel);
    UNUSED(packet_size);

    hid_channel_info_t * pFindInfo =
        updateChannelInfo( channel, hid_channel_type_device );
    
    switch (packet_type){
        case HCI_EVENT_PACKET:
            switch (hci_event_packet_get_type(packet)){
                case BTSTACK_EVENT_STATE:
                    if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) {
                        return;
                    }
                    printf( "%s: initialize HID DEVICE\n", __func__ );
                    s_hid_info_out.state = HID_STATE_NOT_CONNECTED;
                    //s_hid_info_in.state = HID_STATE_NOT_CONNECTED;
                    break;

                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    // ssp: inform about user confirmation request
                    printf("SSP User Confirmation Request with numeric value '%06"PRIu32"'\n",
                           hci_event_user_confirmation_request_get_numeric_value(packet));
                    printf("SSP User Confirmation Auto accept\n");                   
                    break; 

            case HCI_EVENT_HID_META:
                processHidMetaDevice( pFindInfo, channel, packet );
                    break;
                default:
                    break;
            case HCI_EVENT_PIN_CODE_REQUEST:
                // inform about pin code request
                {
                    bd_addr_t event_addr;

                    printf("Pin code request - using '0000'\n");
                    hci_event_pin_code_request_get_bd_addr(packet, event_addr);
                    gap_pin_code_response(event_addr, "0000");
                }
                break;
            }
            break;
        default:
            break;
    }
}



static void processHidMetaHost(
    hid_channel_info_t * pChannelInfo,
    uint16_t channel, uint8_t * packet, uint16_t size )
{
    console_hid_packet_handler_meta_bt( channel, packet, bt_roleType_device );
    
    uint16_t cid = hid_subevent_connection_opened_get_hid_cid(packet);
    uint8_t subevent = hci_event_hid_meta_get_subevent_code(packet);

    uint8_t   status;
    
    switch ( subevent ){
    case HID_SUBEVENT_INCOMING_CONNECTION:
        {
            // デバイスから接続要求が来た時。
            bd_addr_t addr;
            hid_subevent_incoming_connection_get_address( packet, addr );
            ESP_LOGI( TAG, "HID_SUBEVENT_INCOMING_CONNECTION: %s",
                      bd_addr_to_str( addr ) );
            // 接続を許可する場合
            hid_host_accept_connection( cid, s_hid_host_report_mode );
            // 接続を拒否する場合
            // hid_host_decline_connection( cid );
        }
        break;
    case HID_SUBEVENT_CONNECTION_OPENED:
        status = hid_subevent_connection_opened_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
            printf("Connection failed, status 0x%x\n", status);
            s_hid_info_in.state = HID_STATE_NOT_CONNECTED;
            s_hid_info_in.cid = 0;
            return;
        }

        s_hid_info_in.state = HID_STATE_CONNECTED;
        hid_subevent_connection_opened_get_bd_addr( packet, s_hid_info_in.addr );
        /* s_hid_host_descriptor_available = false; */
        s_hid_info_in.cid = hid_subevent_connection_opened_get_hid_cid(packet);
        pChannelInfo->state = CHANNEL_STATE_CONNECTED;

        state_connectedFromDevice( true );
        
        printf("HID Host connected.\n");
        break;

    case HID_SUBEVENT_DESCRIPTOR_AVAILABLE:
        status = hid_subevent_descriptor_available_get_status(packet);
        if (status == ERROR_CODE_SUCCESS){
            /* s_hid_host_descriptor_available = true; */
            printf("HID Descriptor available, please start typing.\n");
        } else {
            printf("Cannot handle input report, HID Descriptor is not available.\n");
        }

        {
            uint16_t cid = hid_subevent_descriptor_available_get_hid_cid( packet );
            log_info_hexdump(
                hid_descriptor_storage_get_descriptor_data(cid),
                hid_descriptor_storage_get_descriptor_len(cid) );
        }
        
        break;

    case HID_SUBEVENT_REPORT:
        /* if (s_hid_host_descriptor_available) */
        {
            const uint8_t * report = hid_subevent_report_get_report(packet);
            const uint16_t reportLen = hid_subevent_report_get_report_len(packet);
            if ( console_get_config_mode() == my_config_mode_setup ) {
                log_info_hexdump( report, reportLen );
            }

            if ( IsInputDeviceConnected() &&
                 report[ 0 ] == 0xA1 && report[ 1 ] == 1 )
            {
                HIDReport_t convRepo;
                my_kbd_conv( report + 2, reportLen - 2, &convRepo );
                if ( console_get_config_mode() == my_config_mode_setup ) {
                    log_info_hexdump( convRepo.codes, sizeof( convRepo.codes ) );
                }
                my_kbd_add_report( &convRepo );

                bt_kb_request_sendKey();
            }
        }
        /* else { */
        /*     printf_hexdump(hid_subevent_report_get_report(packet), */
        /*                    hid_subevent_report_get_report_len(packet)); */
        /* } */
        break;

    case HID_SUBEVENT_SET_PROTOCOL_RESPONSE:
        status = hid_subevent_set_protocol_response_get_handshake_status(packet);
        if (status != HID_HANDSHAKE_PARAM_TYPE_SUCCESSFUL){
            printf("Error set protocol, status 0x%02x\n", status);
            break;
        }
        switch ((hid_protocol_mode_t)hid_subevent_set_protocol_response_get_protocol_mode(packet)){
        case HID_PROTOCOL_MODE_BOOT:
            printf("Protocol mode set: BOOT.\n");
            break;  
        case HID_PROTOCOL_MODE_REPORT:
            printf("Protocol mode set: REPORT.\n");
            break;
        default:
            printf("Unknown protocol mode.\n");
            break; 
        }
        break;

    case HID_SUBEVENT_CONNECTION_CLOSED:
        // The connection was closed.
        hid_host_disconnect( s_hid_info_in.cid );
        s_hid_info_in.cid = 0;
        //s_hid_host_descriptor_available = false;
        pChannelInfo->state = CHANNEL_STATE_RESERVE;
        memset( s_hid_info_in.addr, 0, sizeof( bd_addr_t ) );

        state_connectedFromDevice( false );
        
        printf("HID Host disconnected.\n");

        
        break;
                        
    default:
        break;
    }
}

static void packet_handler_host(
    uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    /* LISTING_PAUSE */
    UNUSED(channel);
    UNUSED(size);

    hid_channel_info_t * pFindInfo =
        updateChannelInfo( channel, hid_channel_type_host );

    uint8_t   event;
    bd_addr_t event_addr;

    /* LISTING_RESUME */
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        event = hci_event_packet_get_type(packet);
            
        switch (event) {            
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING){
                printf( "%s: initialize HID HOST\n", __func__ );
                s_hid_info_in.state = HID_STATE_NOT_CONNECTED;
            }
            break;
            /* LISTING_PAUSE */
        case HCI_EVENT_PIN_CODE_REQUEST:
            // inform about pin code request
            printf("Pin code request - using '0000'\n");
            hci_event_pin_code_request_get_bd_addr(packet, event_addr);
            gap_pin_code_response(event_addr, "0000");
            break;

        case HCI_EVENT_USER_CONFIRMATION_REQUEST:
            // inform about user confirmation request
            printf("SSP User Confirmation Request with numeric value '%"PRIu32"'\n", little_endian_read_32(packet, 8));
            printf("SSP User Confirmation Auto accept\n");
            break;

            /* LISTING_RESUME */
        case HCI_EVENT_HID_META:
            processHidMetaHost( pFindInfo, channel, packet, size );
            break;
        default:
            printf("event default %X\n", event );
            break;
        }
        break;
    default:
        break;
    }
}



static uint8_t s_hid_descriptor_storage[300];

static void packet_handler(
    uint8_t packet_type, uint16_t channel,
    uint8_t * packet, uint16_t packet_size)
{
    const hid_channel_info_t * pFindInfo =
        updateChannelInfo( channel, hid_channel_type_none );

    switch ( pFindInfo->type ) {
    case hid_channel_type_host:
        packet_handler_host( packet_type, channel, packet, packet_size );
        break;
    case hid_channel_type_device:
        packet_handler_device( packet_type, channel, packet, packet_size );
        break;
    default:
        {
            //printf("%s: not found channel -- %d\n", __func__, channel );
            //packet_handler_device( packet_type, channel, packet, packet_size );
        }
        break;
    }
}


void key_hid_host_setup(void)
{
    hid_host_init(s_hid_descriptor_storage, sizeof(s_hid_descriptor_storage));
    hid_host_register_packet_handler(packet_handler_host);

    gap_set_default_link_policy_settings(
        LM_LINK_POLICY_ENABLE_SNIFF_MODE | LM_LINK_POLICY_ENABLE_ROLE_SWITCH);

    hci_set_master_slave_policy(HCI_ROLE_MASTER);

    // register for HCI events
    /* s_hci_event_callback_registration.callback = &packet_handler; */
    /* hci_add_event_handler(&s_hci_event_callback_registration); */

    
    // Disable stdout buffering
    setbuf(stdout, NULL);
}


static void hid_keyboard_setup( void ) {

    gap_set_class_of_device( bt_kb_getCod() );
    gap_set_local_name( s_hid_device_name );
    
    gap_set_default_link_policy_settings(
        LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
    gap_set_allow_role_switch(true);


    memset(s_hid_service_buffer, 0, sizeof(s_hid_service_buffer));

    const uint8_t * pHidDescriptor = get_hidDescriptor();
    uint16_t hidDescriptorLen = get_hidDescriptorLen();

    bool hid_boot_device = true;
        
    hid_sdp_record_t hid_params = {
        bt_kb_getCod(),
        // hid counntry code 33 US
        33,
        // hid_virtual_cable
        0,
        // hid_remote_wake
        1,
        // hid_reconnect_initiate
        1,
        // hid_normally_connectable
        1,
        hid_boot_device,
        // host_max_latency
        1600,
        // host_min_timeout,
        3200,
        // hid_supervision_timeout
        3200,
        pHidDescriptor,
        hidDescriptorLen,
        s_hid_device_name
    };
    
    hid_create_sdp_record(s_hid_service_buffer, 0x10001, &hid_params);

    sdp_register_service(s_hid_service_buffer);

    device_id_create_sdp_record(
        s_device_id_sdp_service_buffer, 0x10003,
        DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH,
        BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);

    sdp_register_service(s_device_id_sdp_service_buffer);

    hid_device_init(hid_boot_device, hidDescriptorLen, pHidDescriptor );

    s_hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&s_hci_event_callback_registration);

    hid_device_register_packet_handler(&packet_handler_device);

}

void key_hid_device_setup( void ) {
    if ( console_get_hid_device_mode() == hid_device_mode_bt ) {
        hid_keyboard_setup();
    } else {
        le_keyboard_setup();
    }
}

void bt_hid_device_request_report( void ) {
    hid_device_request_can_send_now_event( s_hid_info_out.cid );
}

int btstack_main(int argc, const char * argv[]){
    l2cap_init();

#ifdef ENABLE_BLE
    sm_init();
#endif

    sdp_init();
    

    /* key_hid_device_setup(); */

    /* // HID Device の後に HID Host の初期化を行なう。 */
    /* // こうしないと、hid_host_init() で実行している次の処理が */
    /* // hid_device_init() と被って正常に動作しなくなる。 */
    /* // l2cap_register_service(hid_host_packet_handler, PSM_HID_INTERRUPT, 0xffff, gap_get_security_level()); */
    /* key_hid_host_setup(); */
    
    hci_power_control(HCI_POWER_ON);

    return 0;
}

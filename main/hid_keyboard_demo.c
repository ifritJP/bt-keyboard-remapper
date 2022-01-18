/*
 * Copyright (C) 2014 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL BLUEKITCHEN
 * GMBH OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at 
 * contact@bluekitchen-gmbh.com
 *
 */

#define BTSTACK_FILE__ "hid_keyboard_demo.c"

#include "hid_key.h"

hid_device_mode_t s_hid_device_mode = hid_device_mode_bt;

 
// *****************************************************************************
/* EXAMPLE_START(hid_keyboard_demo): HID Keyboard Classic
 *
 * @text This HID Device example demonstrates how to implement
 * an HID keyboard. Without a HAVE_BTSTACK_STDIN, a fixed demo text is sent
 * If HAVE_BTSTACK_STDIN is defined, you can type from the terminal
 */
// *****************************************************************************


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "btstack.h"

#ifdef HAVE_BTSTACK_STDIN
#include "btstack_stdin.h"
#endif

#include "hog_key.h"
#include "console.h"

// to enable demo text on POSIX systems
// #undef HAVE_BTSTACK_STDIN

// When not set to 0xffff, sniff and sniff subrating are enabled
static uint16_t host_max_latency = 1600;
static uint16_t host_min_timeout = 3200;

static hid_protocol_mode_t hid_host_report_mode = HID_PROTOCOL_MODE_REPORT_WITH_FALLBACK_TO_BOOT;
static bool     hid_host_descriptor_available = false;

#define REPORT_ID 0x01

// close to USB HID Specification 1.1, Appendix B.1
const uint8_t hid_descriptor_keyboard[] = {

    0x05, 0x01,                    // Usage Page (Generic Desktop)
    0x09, 0x06,                    // Usage (Keyboard)
    0xa1, 0x01,                    // Collection (Application)

    // Report ID

    0x85, REPORT_ID,               // Report ID

    // Modifier byte (input)

    0x05, 0x07,                    //   Usage Page (Key Codes)
    0x75, 0x01,                    //   Report Size (1)
    0x95, 0x08,                    //   Report Count (8)
    0x05, 0x07,                    //   Usage Page (Key codes)
    0x19, 0xe0,                    //   Usage Minimum (Keyboard LeftControl)
    0x29, 0xe7,                    //   Usage Maxium (Keyboard Right GUI)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0x01,                    //   Logical Maximum (1)
    0x81, 0x02,                    //   Input (Data, Variable, Absolute)

    // Reserved byte (input)

    0x75, 0x01,                    //   Report Size (1)
    0x95, 0x08,                    //   Report Count (8)
    0x81, 0x03,                    //   Input (Constant, Variable, Absolute)

    // LED report + padding (output)

    0x95, 0x05,                    //   Report Count (5)
    0x75, 0x01,                    //   Report Size (1)
    0x05, 0x08,                    //   Usage Page (LEDs)
    0x19, 0x01,                    //   Usage Minimum (Num Lock)
    0x29, 0x05,                    //   Usage Maxium (Kana)
    0x91, 0x02,                    //   Output (Data, Variable, Absolute)

    0x95, 0x01,                    //   Report Count (1)
    0x75, 0x03,                    //   Report Size (3)
    0x91, 0x03,                    //   Output (Constant, Variable, Absolute)

    // Keycodes (input)

    0x95, 0x06,                    //   Report Count (6)
    0x75, 0x08,                    //   Report Size (8)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0xff,                    //   Logical Maximum (1)
    0x05, 0x07,                    //   Usage Page (Key codes)
    0x19, 0x00,                    //   Usage Minimum (Reserved (no event indicated))
    0x29, 0xff,                    //   Usage Maxium (Reserved)
    0x81, 0x00,                    //   Input (Data, Array)

    0xc0,                          // End collection  
};

const uint8_t hid_descriptor_keyboard2[] = {
    0x05, 0x01,
    0x09, 0x06,
    0xa1, 0x01,
    0x05, 0x07,
    0x19, 0xe0,
    0x29, 0xe7,
    0x15, 0x00,
    0x25, 0x01,
    0x75, 0x01,
    0x95, 0x08,
    0x81, 0x02,
    0x95, 0x01,
    0x75, 0x08,
    0x81, 0x01,
    0x95, 0x05,
    0x75, 0x01,
    0x05, 0x08,
    0x19, 0x01,
    0x29, 0x05,
    0x91, 0x02,
    0x95, 0x01,
    0x75, 0x03,
    0x91, 0x01,
    0x95, 0x06,
    0x75, 0x08,
    0x15, 0x00,
    0x25, 0x65,
    0x05, 0x07,
    0x19, 0x00,
    0x29, 0x65,
    0x81, 0x00,

    0xc0    
};

// 
#define CHAR_ILLEGAL     0xff
#define CHAR_RETURN     '\n'
#define CHAR_ESCAPE      27
#define CHAR_TAB         '\t'
#define CHAR_BACKSPACE   0x7f

// Simplified US Keyboard with Shift modifier

/**
 * English (US)
 */
static const uint8_t keytable_us_none [] = {
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /*   0-3 */
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',                   /*  4-13 */
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',                   /* 14-23 */
    'u', 'v', 'w', 'x', 'y', 'z',                                       /* 24-29 */
    '1', '2', '3', '4', '5', '6', '7', '8', '9', '0',                   /* 30-39 */
    CHAR_RETURN, CHAR_ESCAPE, CHAR_BACKSPACE, CHAR_TAB, ' ',            /* 40-44 */
    '-', '=', '[', ']', '\\', CHAR_ILLEGAL, ';', '\'', 0x60, ',',       /* 45-54 */
    '.', '/', CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,   /* 55-60 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 61-64 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 65-68 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 69-72 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 73-76 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 77-80 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 81-84 */
    '*', '-', '+', '\n', '1', '2', '3', '4', '5',                       /* 85-97 */
    '6', '7', '8', '9', '0', '.', 0xa7,                                 /* 97-100 */
}; 

static const uint8_t keytable_us_shift[] = {
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /*  0-3  */
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',                   /*  4-13 */
    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',                   /* 14-23 */
    'U', 'V', 'W', 'X', 'Y', 'Z',                                       /* 24-29 */
    '!', '@', '#', '$', '%', '^', '&', '*', '(', ')',                   /* 30-39 */
    CHAR_RETURN, CHAR_ESCAPE, CHAR_BACKSPACE, CHAR_TAB, ' ',            /* 40-44 */
    '_', '+', '{', '}', '|', CHAR_ILLEGAL, ':', '"', 0x7E, '<',         /* 45-54 */
    '>', '?', CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,   /* 55-60 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 61-64 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 65-68 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 69-72 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 73-76 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 77-80 */
    CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL, CHAR_ILLEGAL,             /* 81-84 */
    '*', '-', '+', '\n', '1', '2', '3', '4', '5',                       /* 85-97 */
    '6', '7', '8', '9', '0', '.', 0xb1,                                 /* 97-100 */
}; 

// STATE

static uint8_t hid_service_buffer[300];
static uint8_t device_id_sdp_service_buffer[100];
static const char hid_device_name[] = "BTstack HID Keyboard";
static btstack_packet_callback_registration_t hci_event_callback_registration;
static uint8_t hid_boot_device = 0;

#ifdef HAVE_BTSTACK_STDIN
//static const char * device_addr_string = "BC:EC:5D:E6:15:03";
static const char * device_addr_string = "EC:00:FE:00:09:56";
#endif

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

void key_hid_set_device_mode( hid_device_mode_t mode ) {
    s_hid_device_mode = mode;
}

uint8_t key_hid_host_connect( bd_addr_t addr ) {
    return hid_host_connect( addr, hid_host_report_mode, &s_hid_info_in.cid);
}

static bd_addr_t s_connecting_device_target_addr;
void key_hid_clear_device_target_addr( void ) {
    memset( s_connecting_device_target_addr, 0, sizeof( bd_addr_t ) );
}

void key_hid_device_connect( bd_addr_t addr ) {
    memcpy( s_connecting_device_target_addr, addr, sizeof( bd_addr_t ) );
    hid_device_connect( addr, &s_hid_info_in.cid );
}

// HID Keyboard lookup
static int lookup_keycode(uint8_t character, const uint8_t * table, int size, uint8_t * keycode){
    int i;
    for (i=0;i<size;i++){
        if (table[i] != character) continue;
        *keycode = i;
        return 1;
    }
    return 0;
}

static int keycode_and_modifer_us_for_character(uint8_t character, uint8_t * keycode, uint8_t * modifier){
    int found;
    found = lookup_keycode(character, keytable_us_none, sizeof(keytable_us_none), keycode);
    if (found) {
        *modifier = 0;  // none
        return 1;
    }
    found = lookup_keycode(character, keytable_us_shift, sizeof(keytable_us_shift), keycode);
    if (found) {
        *modifier = 2;  // shift
        return 1;
    }
    return 0;
}

// HID Report sending
static uint8_t s_report[ 10 ] = {0xa1, REPORT_ID, };
static bool s_hasSendKey = false;
static bool s_autoRelease = false;

static bool IsInputDeviceConnected( void ) {
    if ( s_hid_device_mode == hid_device_mode_bt ) {
        return s_hid_info_out.cid != 0;
    } else {
        return hog_isConnected();
    }
}

static void send_key(int modifier, int keycode){
    if ( s_hid_info_out.cid == 0 ) {
        return;
    }
    
    s_report[ 2 ] = modifier;
    s_report[ 4 ] = keycode;
    s_hasSendKey = true;
    s_autoRelease = true;
    
    hid_device_request_can_send_now_event(s_hid_info_out.cid);
    printf( "%s: key = 0x%X\n", __func__, keycode );
}

// Demo Application

//#ifdef HAVE_BTSTACK_STDIN

// On systems with STDIN, we can directly type on the console

static void stdin_process(char character){
    uint8_t modifier;
    uint8_t keycode;
    int found;

    //Console_input( character );
    return;
    
    printf("%s: %d, %d, 0x%X\n", __func__, s_hid_info_in.state, character, character );

    if ( character == '!' ) {
        bd_addr_t addr;
        // 接続先ホストの Bluetooth Address を指定する
        sscanf_bd_addr( "00:1B:DC:06:61:EB", addr );
        printf( "hid_device_connect\n" );
        // pairing 済みの時の接続
        hid_device_connect( addr, &s_hid_info_in.cid );
        return;
    }

    if ( s_hid_info_out.state == HID_STATE_CONNECTED && character == 'j' ) {
        found = keycode_and_modifer_us_for_character(character, &keycode, &modifier);
        printf("%s: found = %d, %d, %X\n", __func__, found, keycode, modifier );
        if (found){
            send_key(modifier, keycode);
            return;
        }
    }

    switch (s_hid_info_in.state ) {
        case HID_STATE_BOOTING:
        case HID_STATE_CONNECTED:
            found = keycode_and_modifer_us_for_character(character, &keycode, &modifier);
            printf("%s: found = %d, %d, %X\n", __func__, found, keycode, modifier );
            if (found){
                send_key(modifier, keycode);
                return;
            }
            break;
        case HID_STATE_NOT_CONNECTED:
            printf("Connecting to %s...\n", device_addr_string );
            //hid_device_connect(device_addr, &s_hid_info_in.cid);

            bd_addr_t addr;
            sscanf_bd_addr( device_addr_string, addr );
            uint8_t status = hid_host_connect(
                addr, hid_host_report_mode, &s_hid_info_in.cid);
            if (status != ERROR_CODE_SUCCESS){
                printf("HID host failed, status 0x%02x\n", status);
            }
            break;
        default:
            btstack_assert(false);
            break;
    }
}
//#else

// On embedded systems, send constant demo text with fixed period

#define TYPING_PERIOD_MS 100
static char s_sendText[ 50 ];
static int s_sendTextPos = 0;

static btstack_timer_source_t typing_timer;

static void typing_timer_handler(btstack_timer_source_t * ts){

    // abort if not connected
    if (!s_hid_info_out.cid) return;

    // get next character
    uint8_t character = s_sendText[s_sendTextPos++];
    if (s_sendText[s_sendTextPos] == 0){
        btstack_run_loop_remove_timer( ts );
        s_sendTextPos = 0;
    } else {
        // set next timer
        btstack_run_loop_set_timer(ts, TYPING_PERIOD_MS);
        btstack_run_loop_add_timer(ts);
    }

    // get keycodeand send
    uint8_t modifier;
    uint8_t keycode;
    int found = keycode_and_modifer_us_for_character(character, &keycode, &modifier);
    if (found){
        send_key(modifier, keycode);
    }
}

void hid_embedded_start_typing( const char * pTxt ){
    int len = sizeof( s_sendText ) - 1;
    strncpy( s_sendText, pTxt, len );
    s_sendText[ len ] = '\0';
    
    s_sendTextPos = 0;
    // set one-shot timer
    typing_timer.process = &typing_timer_handler;
    btstack_run_loop_set_timer(&typing_timer, TYPING_PERIOD_MS);
    btstack_run_loop_add_timer(&typing_timer);
}

//#endif

static void processHidMeta(
    hid_channel_info_t * pChannelInfo, uint16_t channel, uint8_t * packet )
{
    uint16_t cid = hid_subevent_connection_opened_get_hid_cid(packet);
    uint8_t subevent = hci_event_hid_meta_get_subevent_code(packet);
    printf( "%s: channel = %d, %d, %d\n",
            __func__, channel, cid, subevent );
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
            pInfo->state = HID_STATE_CONNECTED;
            hid_subevent_connection_opened_get_bd_addr( packet, pInfo->addr );
            
            pInfo->cid = cid;
            pInfo->channel = channel;

            pChannelInfo->state = CHANNEL_STATE_CONNECTED;
            
#ifdef HAVE_BTSTACK_STDIN                        
            printf("HID Connected, please start typing...\n");
#else                        
            printf("HID Connected, sending demo text...\n");
            hid_embedded_start_typing();
#endif
        }
        break;
    case HID_SUBEVENT_CONNECTION_CLOSED:
        printf("HID Disconnected\n");
                            
        hid_info_t * pInfo = &s_hid_info_out;
        memset( pInfo->addr, 0, sizeof( bd_addr_t ) );
        pInfo->state = HID_STATE_NOT_CONNECTED;
        pInfo->cid = 0;
        pChannelInfo->state = CHANNEL_STATE_RESERVE;
        break;
    case HID_SUBEVENT_CAN_SEND_NOW:
        printf("CAN_SEND_NOW\n");
        if ( !IsInputDeviceConnected() ) {
            return;
        }
        if ( s_hid_device_mode == hid_device_mode_bt ) {
            if (s_hasSendKey){
                s_hasSendKey = false;
                hid_device_send_interrupt_message(
                    s_hid_info_out.cid, s_report, sizeof(s_report));
                if ( s_autoRelease ) {
                    hid_device_request_can_send_now_event(s_hid_info_out.cid);
                }
            } else {
                if ( s_autoRelease ) {
                    s_autoRelease = false;
                    memset( s_report + 2, 0, 8 );
                    hid_device_send_interrupt_message(
                        s_hid_info_out.cid, s_report, sizeof(s_report));
                }
            }
        } else {
            
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
                    if (btstack_event_state_get_state(packet) != HCI_STATE_WORKING) return;
                    printf( "%s: initialize HID DEVICE\n", __func__ );
                    s_hid_info_out.state = HID_STATE_NOT_CONNECTED;
                    //s_hid_info_in.state = HID_STATE_NOT_CONNECTED;
                    break;

                case HCI_EVENT_USER_CONFIRMATION_REQUEST:
                    // ssp: inform about user confirmation request
                    printf("SSP User Confirmation Request with numeric value '%06"PRIu32"'\n", hci_event_user_confirmation_request_get_numeric_value(packet));
                    printf("SSP User Confirmation Auto accept\n");                   
                    break; 

            case HCI_EVENT_HID_META:
                processHidMeta( pFindInfo, channel, packet );
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



#define NUM_KEYS 6
static uint8_t last_keys[NUM_KEYS];
static void hid_host_handle_interrupt_report(const uint8_t * report, uint16_t report_len){
    // check if HID Input Report
    if (report_len < 1) return;
    if (*report != 0xa1) return; 
    
    report++;
    report_len--;
    
    btstack_hid_parser_t parser;
    btstack_hid_parser_init(&parser, 
                            hid_descriptor_storage_get_descriptor_data(s_hid_info_in.cid), 
                            hid_descriptor_storage_get_descriptor_len(s_hid_info_in.cid), 
                            HID_REPORT_TYPE_INPUT, report, report_len);

    int shift = 0;
    uint8_t new_keys[NUM_KEYS];
    memset(new_keys, 0, sizeof(new_keys));
    int     new_keys_count = 0;
    while (btstack_hid_parser_has_more(&parser)){
        uint16_t usage_page;
        uint16_t usage;
        int32_t  value;
        btstack_hid_parser_get_field(&parser, &usage_page, &usage, &value);
        if (usage_page != 0x07) continue;   
        switch (usage){
        case 0xe1:
        case 0xe6:
            if (value){
                shift = 1;
            }
            continue;
        case 0x00:
            continue;
        default:
            break;
        }
        if (usage >= sizeof(keytable_us_none)) continue;

        // store new keys
        new_keys[new_keys_count++] = usage;

        // check if usage was used last time (and ignore in that case)
        int i;
        for (i=0;i<NUM_KEYS;i++){
            if (usage == last_keys[i]){
                usage = 0;
            }
        }
        if (usage == 0) continue;

        uint8_t key;
        if (shift){
            key = keytable_us_shift[usage];
        } else {
            key = keytable_us_none[usage];
        }
        if (key == CHAR_ILLEGAL) continue;
        if (key == CHAR_BACKSPACE){ 
            printf("\b \b");    // go back one char, print space, go back one char again
            continue;
        }
        printf("%c", key);
    }
    memcpy(last_keys, new_keys, NUM_KEYS);
}


static void processHidMetaHost(
    hid_channel_info_t * pChannelInfo,
    uint16_t channel, uint8_t * packet, uint16_t size )
{
    uint16_t cid = hid_subevent_connection_opened_get_hid_cid(packet);
    uint8_t subevent = hci_event_hid_meta_get_subevent_code(packet);
    printf( "%s: channel = %d, %d, %d, %d\n", __func__, channel, cid, subevent, size );

    uint8_t   status;
    
    switch ( subevent ){
    case HID_SUBEVENT_INCOMING_CONNECTION:
        {
            // There is an incoming connection: we can accept it or decline it.
            // The hid_host_report_mode in the hid_host_accept_connection function 
            // allows the application to request a protocol mode. 
            // For available protocol modes, see hid_protocol_mode_t in btstack_hid.h file.
           
            hid_host_accept_connection( cid, hid_host_report_mode );
        }
        
        break;
                        
    case HID_SUBEVENT_CONNECTION_OPENED:
        // The status field of this event indicates if the control and interrupt
        // connections were opened successfully.
        status = hid_subevent_connection_opened_get_status(packet);
        if (status != ERROR_CODE_SUCCESS) {
            printf("Connection failed, status 0x%x\n", status);
            s_hid_info_in.state = HID_STATE_NOT_CONNECTED;
            s_hid_info_in.cid = 0;
            return;
        }

        //hid_device_disconnect(uint16_t hid_cid)
        
        s_hid_info_in.state = HID_STATE_CONNECTED;
        hid_subevent_connection_opened_get_bd_addr( packet, s_hid_info_in.addr );
        hid_host_descriptor_available = false;
        s_hid_info_in.cid = hid_subevent_connection_opened_get_hid_cid(packet);
        pChannelInfo->state = CHANNEL_STATE_CONNECTED;
        
        printf("HID Host connected.\n");
        break;

    case HID_SUBEVENT_DESCRIPTOR_AVAILABLE:
        // This event will follows HID_SUBEVENT_CONNECTION_OPENED event. 
        // For incoming connections, i.e. HID Device initiating the connection,
        // the HID_SUBEVENT_DESCRIPTOR_AVAILABLE is delayed, and some HID  
        // reports may be received via HID_SUBEVENT_REPORT event. It is up to 
        // the application if these reports should be buffered or ignored until 
        // the HID descriptor is available.
        status = hid_subevent_descriptor_available_get_status(packet);
        if (status == ERROR_CODE_SUCCESS){
            hid_host_descriptor_available = true;
            printf("HID Descriptor available, please start typing.\n");
        } else {
            printf("Cannot handle input report, HID Descriptor is not available.\n");
        }
        break;

    case HID_SUBEVENT_REPORT:
        // Handle input report.
        if (hid_host_descriptor_available){
            hid_host_handle_interrupt_report(
                hid_subevent_report_get_report(packet),
                hid_subevent_report_get_report_len(packet));

            const uint8_t * report = hid_subevent_report_get_report(packet);
            const uint16_t reportLen = hid_subevent_report_get_report_len(packet);
            printf( "%s: reportLen = %d\n", __func__, reportLen );
            log_info_hexdump( report, reportLen );

            if ( IsInputDeviceConnected() &&
                 report[ 0 ] == 0xA1 && report[ 1 ] == 1 )
            {
                memset( s_report + 2, 0, sizeof( s_report ) - 2 );
                memcpy( s_report + 2, report + 2, reportLen - 2 );
                if ( s_hid_device_mode == hid_device_mode_bt ) {
                    printf( "send %d, %d\n", s_report[ 2 ], s_report[ 4 ] );
                    s_hasSendKey = true;
                    hid_device_request_can_send_now_event(s_hid_info_out.cid);
                } else {
                    printf( "send ble %d, %d\n", s_report[ 2 ], s_report[ 4 ] );
                    hog_send_report( &s_report[ 2 ], reportLen - 2 );
                }
            }
        } else {
            printf_hexdump(hid_subevent_report_get_report(packet),
                           hid_subevent_report_get_report_len(packet));
        }
        break;

    case HID_SUBEVENT_SET_PROTOCOL_RESPONSE:
        // For incoming connections, the library will set the protocol mode of the
        // HID Device as requested in the call to hid_host_accept_connection. The event 
        // reports the result. For connections initiated by calling hid_host_connect, 
        // this event will occur only if the established report mode is boot mode.
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
        s_hid_info_in.cid = 0;
        hid_host_descriptor_available = false;
        pChannelInfo->state = CHANNEL_STATE_RESERVE;
        memset( s_hid_info_in.addr, 0, sizeof( bd_addr_t ) );
        
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

    printf("packet %X, %d, %d\n", packet_type, channel, size);
    
    /* LISTING_RESUME */
    switch (packet_type) {
    case HCI_EVENT_PACKET:
        event = hci_event_packet_get_type(packet);
        printf("event %X\n", event );
            
        switch (event) {            
#ifndef HAVE_BTSTACK_STDIN
            /* @text When BTSTACK_EVENT_STATE with state HCI_STATE_WORKING
             * is received and the example is started in client mode, the remote SDP HID query is started.
             */
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING){
                printf( "%s: initialize HID\n", __func__ );
                uint8_t status = hid_host_connect(remote_addr, hid_host_report_mode, &s_hid_info_in.cid);
                if (status != ERROR_CODE_SUCCESS){
                    printf("HID host connect failed, status 0x%02x.\n", status);
                }
            }
            break;
#else
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING){
                printf( "%s: initialize HID HOST\n", __func__ );
                s_hid_info_in.state = HID_STATE_NOT_CONNECTED;
            }
            break;
#endif
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



static void hid_set_report_callback(uint16_t cid, hid_report_type_t report_type, int report_size, uint8_t * report){
    UNUSED(cid);
    UNUSED(report_type);
    UNUSED(report_size);
    UNUSED(report);
    printf("set report\n");
}

static void hid_report_data_callback(uint16_t cid, hid_report_type_t report_type, uint16_t report_id, int report_size, uint8_t * report){
    UNUSED(cid);
    UNUSED(report_type);
    UNUSED(report_id);
    UNUSED(report_size);
    UNUSED(report);
    printf("do smth with report: %d, %d, %d, %d\n",
           cid, report_type, report_id, report_size);
}

#define MAX_ATTRIBUTE_VALUE_SIZE 300

static uint8_t hid_descriptor_storage[MAX_ATTRIBUTE_VALUE_SIZE];

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

    // // Initialize L2CAP
    //l2cap_init();

/* #ifdef ENABLE_BLE */
/*     // Initialize LE Security Manager. Needed for cross-transport key derivation */
/*     sm_init(); */
/* #endif */

    // Initialize HID Host
    hid_host_init(hid_descriptor_storage, sizeof(hid_descriptor_storage));
    hid_host_register_packet_handler(packet_handler_host);

    /* // Allow sniff mode requests by HID device and support role switch */
    /* gap_set_default_link_policy_settings(LM_LINK_POLICY_ENABLE_SNIFF_MODE | LM_LINK_POLICY_ENABLE_ROLE_SWITCH); */

    // try to become master on incoming connections
    hci_set_master_slave_policy(HCI_ROLE_MASTER);

    // Disable stdout buffering
    setbuf(stdout, NULL);
}


static void hid_keyboard_setup( void ) {

    /* // allow to get found by inquiry */
    /* gap_discoverable_control(1); */

    
    // use Limited Discoverable Mode; Peripheral; Keyboard as CoD

    // http://bluetooth-pentest.narod.ru/software/bluetooth_class_of_device-service_generator.html
    // Limited Discoverable Mode
    // Peripheral
    // Keyboard

    uint32_t class_of_device = 0x2540;
    gap_set_class_of_device( class_of_device );

    // set local name to be identified - zeroes will be replaced by actual BD ADDR
    gap_set_local_name("BT Keyboard Remapper 00:00:00:00:00:00");
    // allow for role switch in general and sniff mode
    gap_set_default_link_policy_settings(
        LM_LINK_POLICY_ENABLE_ROLE_SWITCH | LM_LINK_POLICY_ENABLE_SNIFF_MODE );
    // allow for role switch on outgoing connections - this allow HID Host to become master when we re-connect to it
    gap_set_allow_role_switch(true);


/*     // L2CAP */
/*     l2cap_init(); */

/* #ifdef ENABLE_BLE */
/*     // Initialize LE Security Manager. Needed for cross-transport key derivation */
/*     sm_init(); */
/* #endif */

/*     // SDP Server */
/*     sdp_init(); */

    
    memset(hid_service_buffer, 0, sizeof(hid_service_buffer));

    uint8_t hid_virtual_cable = 0;
    uint8_t hid_remote_wake = 1;
    uint8_t hid_reconnect_initiate = 1;
    uint8_t hid_normally_connectable = 1;


    const uint8_t * pHidDescriptor = hid_descriptor_keyboard;
    uint16_t hidDescriptorLen = sizeof( hid_descriptor_keyboard );
    
        
    hid_sdp_record_t hid_params = {
        // hid sevice subclass 2540 Keyboard, hid counntry code 33 US
        0x2540, 33, 
        hid_virtual_cable, hid_remote_wake, 
        hid_reconnect_initiate, hid_normally_connectable,
        hid_boot_device,
        host_max_latency, host_min_timeout,
        3200,
        pHidDescriptor,
        hidDescriptorLen,
        hid_device_name
    };
    
    hid_create_sdp_record(hid_service_buffer, 0x10001, &hid_params);

    printf("HID service record size: %u\n", de_get_len( hid_service_buffer));
    sdp_register_service(hid_service_buffer);

    // See https://www.bluetooth.com/specifications/assigned-numbers/company-identifiers if you don't have a USB Vendor ID and need a Bluetooth Vendor ID
    // device info: BlueKitchen GmbH, product 1, version 1
    device_id_create_sdp_record(
        device_id_sdp_service_buffer, 0x10003,
        DEVICE_ID_VENDOR_ID_SOURCE_BLUETOOTH,
        BLUETOOTH_COMPANY_ID_BLUEKITCHEN_GMBH, 1, 1);
    printf("Device ID SDP service record size: %u\n", de_get_len((uint8_t*)device_id_sdp_service_buffer));
    sdp_register_service(device_id_sdp_service_buffer);

    // HID Device
    hid_device_init(hid_boot_device, hidDescriptorLen, pHidDescriptor );

    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    // register for HID events
    hid_device_register_packet_handler(&packet_handler_device);

    hid_device_register_set_report_callback(&hid_set_report_callback);
    hid_device_register_report_data_callback(&hid_report_data_callback);
}

void key_hid_device_setup( void ) {
    if ( s_hid_device_mode == hid_device_mode_bt ) {
        hid_keyboard_setup();
    } else {
        le_keyboard_setup();
    }
}

/* @section Main Application Setup
 *
 * @text Listing MainConfiguration shows main application code. 
 * To run a HID Device service you need to initialize the SDP, and to create and register HID Device record with it. 
 * At the end the Bluetooth stack is started.
 */

/* LISTING_START(MainConfiguration): Setup HID Device */
int btstack_main(int argc, const char * argv[]);
int btstack_main(int argc, const char * argv[]){
    (void)argc;
    (void)argv;

    //Console_init();

    // L2CAP
    l2cap_init();

#ifdef ENABLE_BLE
    // Initialize LE Security Manager. Needed for cross-transport key derivation
    sm_init();
#endif

    // SDP Server
    sdp_init();
    

    /* key_hid_device_setup(); */

    /* // HID Device の後に HID Host の初期化を行なう。 */
    /* // こうしないと、hid_host_init() で実行している次の処理が */
    /* // hid_device_init() と被って正常に動作しなくなる。 */
    /* // l2cap_register_service(hid_host_packet_handler, PSM_HID_INTERRUPT, 0xffff, gap_get_security_level()); */
    /* key_hid_host_setup(); */
    
/* #ifdef HAVE_BTSTACK_STDIN */
/*     //sscanf_bd_addr(device_addr_string, device_addr); */
/*     btstack_stdin_setup(stdin_process); */
/* #endif */
    // turn on!
    hci_power_control(HCI_POWER_ON);

    return 0;
}
/* LISTING_END */
/* EXAMPLE_END */

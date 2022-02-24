#include "hog_keyboard_ctrl.h"
#include "hid_reportDesc.h"

#include "btstack.h"

#include "hog_key.h"
#include "HID.h"
#include "my_keyboard.h"
#include "my_console.h"
#include "stateCtrl.h"

static hci_con_handle_t s_con_handle = HCI_CON_HANDLE_INVALID;
static uint8_t s_protocol_mode = 1;

bool hog_isConnected( void )
{
    return s_con_handle != HCI_CON_HANDLE_INVALID;
}

static void packet_handler (
    uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);

const uint8_t adv_data[] = {
    // Flags general discoverable, BR/EDR not supported
    0x02, BLUETOOTH_DATA_TYPE_FLAGS, 0x06,
    // Name
    0x11, BLUETOOTH_DATA_TYPE_COMPLETE_LOCAL_NAME,
    'B', 'L', 'E', '-', 'K', 'b', 'd', '-', 'R', 'e', 'm', 'a', 'p', 'p', 'e', 'r',
    // 16-bit Service UUIDs
    0x03, BLUETOOTH_DATA_TYPE_COMPLETE_LIST_OF_16_BIT_SERVICE_CLASS_UUIDS,
    ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE & 0xff,
    ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE >> 8,
    // Appearance HID - Keyboard (Category 15, Sub-Category 1)
    0x03, BLUETOOTH_DATA_TYPE_APPEARANCE, 0xC1, 0x03,
};

void le_keyboard_setup(void){

    /* l2cap_init(); */

    /* // setup SM: Display only */
    /* sm_init(); */

    //// キーボード側で特定のキーを入力する
    // sm_set_io_capabilities(IO_CAPABILITY_DISPLAY_ONLY );
    //// ホストとデバイスに番号を表示し、ペアリングするかどうかの確認画面を出す
    // SM_EVENT_NUMERIC_COMPARISON_REQUEST イベントで、
    // sm_numeric_comparison_confirm() をコールし与えられたコードを入力する。
    /* sm_set_io_capabilities(IO_CAPABILITY_DISPLAY_YES_NO); */
    //// sm_passkey_input() を使用して、ホスト側に表示されている特定のキーを入力する
    /* sm_set_io_capabilities(IO_CAPABILITY_KEYBOARD_ONLY); */
    // ペアリングするかどうかの確認画面をホストに出す。
    // SM_EVENT_JUST_WORKS_REQUEST イベントで、
    // sm_just_works_confirm() をコールする。
    sm_set_io_capabilities(IO_CAPABILITY_NO_INPUT_NO_OUTPUT);
    // IO_CAPABILITY_DISPLAY_YES_NO との違いが分からん
    // sm_set_io_capabilities(IO_CAPABILITY_KEYBOARD_DISPLAY);

    sm_set_authentication_requirements(
        SM_AUTHREQ_SECURE_CONNECTION | SM_AUTHREQ_BONDING);
    //sm_set_authentication_requirements(SM_AUTHREQ_NO_BONDING);
    

    // setup ATT server
    att_server_init(profile_data, NULL, NULL);

    // setup battery service
    battery_service_server_init( 100 );

    // setup device information service
    device_information_service_server_init();

    // setup HID Device service
    hids_device_init(0, get_hidDescriptor(), get_hidDescriptorLen() );

    // setup advertisements
    uint16_t adv_int_min = 0x0030;
    uint16_t adv_int_max = 0x0030;
    uint8_t adv_type = 0;
    bd_addr_t null_addr;
    memset(null_addr, 0, sizeof(bd_addr_t));
    gap_advertisements_set_params(
        adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
    gap_advertisements_set_data( sizeof(adv_data), (uint8_t*) adv_data);
    gap_advertisements_enable(1);
    
    // register for SM events
    static btstack_packet_callback_registration_t sm_event_callback_registration;
    sm_event_callback_registration.callback = packet_handler;
    sm_add_event_handler(&sm_event_callback_registration);

    // register for HIDS
    hids_device_register_packet_handler(packet_handler);
}

void hog_send_report( void ) {
    hids_device_request_can_send_now_event( s_con_handle );
}

void hog_send_passkey( uint32_t passkey ) {
    btstack_linked_list_iterator_t it;
    hci_connections_get_iterator(&it);
    while(btstack_linked_list_iterator_has_next(&it)){
        hci_connection_t * connection =
            (hci_connection_t *) btstack_linked_list_iterator_next(&it);
        printf( "%s: %d, %d\n", __func__, connection->role, connection->state );

        sm_passkey_input( connection->con_handle, passkey );
    }

}

static void send_report( void ){
    HIDReport_t report;
    if ( my_kbd_get_report( &report ) ) {
        switch (s_protocol_mode){
        case 0:
            hids_device_send_boot_keyboard_input_report(
                s_con_handle, report.codes, sizeof(report.codes));
            break;
        case 1:
            hids_device_send_input_report(
                s_con_handle, report.codes, sizeof(report.codes));
            break;
        default:
            break;
        }
        if ( my_kbd_has_report() ) {
            hids_device_request_can_send_now_event(s_con_handle);
        }
    }
}

static void packet_handler (
    uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{

    if (packet_type != HCI_EVENT_PACKET) return;

    switch (hci_event_packet_get_type(packet)) {
    case HCI_EVENT_DISCONNECTION_COMPLETE:
        {
            uint16_t handle =
                hci_event_disconnection_complete_get_connection_handle( packet );
            if ( handle == s_con_handle ) {
                s_con_handle = HCI_CON_HANDLE_INVALID;
                printf("Disconnected\n");
                gap_advertisements_enable( 1 );
                state_connectedToHost( false );
            }
        }
        break;
    case HCI_EVENT_HIDS_META:
        console_hid_packet_handler_meta_le(
            channel, packet, bt_roleType_host );

           
        switch (hci_event_hids_meta_get_subevent_code(packet)){
        case HIDS_SUBEVENT_INPUT_REPORT_ENABLE:
            gap_advertisements_enable( 0 );
            state_connectedToHost( true );

                    
            s_con_handle = hids_subevent_input_report_enable_get_con_handle(packet);

            hci_connection_t * connection = hci_connection_for_handle(s_con_handle);
            printf( "addr: %s\n", bd_addr_to_str( connection->address ) );

            printf("Report Characteristic Subscribed %u\n",
                   hids_subevent_input_report_enable_get_enable(packet));
                    
                    
            break;
        case HIDS_SUBEVENT_BOOT_KEYBOARD_INPUT_REPORT_ENABLE:
            s_con_handle = hids_subevent_boot_keyboard_input_report_enable_get_con_handle(packet);
            printf("Boot Keyboard Characteristic Subscribed %u\n",
                   hids_subevent_boot_keyboard_input_report_enable_get_enable(packet));
            break;
        case HIDS_SUBEVENT_PROTOCOL_MODE:
            s_protocol_mode = hids_subevent_protocol_mode_get_protocol_mode(packet);
            printf("Protocol Mode: %s mode\n",
                   hids_subevent_protocol_mode_get_protocol_mode(packet) ? "Report" : "Boot");
            break;
        case HIDS_SUBEVENT_CAN_SEND_NOW:
            send_report();
            break;
        default:
            break;
        }
        break;
            
    default:
        break;
    }
}

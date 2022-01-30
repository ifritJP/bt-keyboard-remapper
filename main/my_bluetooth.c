#include <stdio.h>
#include "bluetooth.h"
#include "btstack.h"
#include "hid_key.h"
#include "btstack/my_gap_inquiry.h"

void bt_kb_init( void ) {
}

void bt_kb_host_connect_to_device( bd_addr_t addr ) {
    printf( "%s: %s\n", __func__, bd_addr_to_str( addr ) );
    uint8_t status = key_hid_host_connect( addr );
    if (status != ERROR_CODE_SUCCESS){
        printf("HID host failed, status 0x%02x\n", status);
    }
}

void bt_kb_device_connect_to_host( bd_addr_t addr ) {
    printf( "%s: %s\n", __func__, bd_addr_to_str( addr ) );
    // pairing 済みの時の接続
    key_hid_device_connect( addr );
}

void bt_kb_set_discoverable( bool flag ) {
    if ( flag ) {
        key_hid_clear_device_target_addr();
    }
    gap_discoverable_control( flag ? 1 : 0 );
}

void bt_kb_set_sendKey( const char * pKeys ) {
    hid_embedded_start_typing( (char *)pKeys );
}


void bt_kb_resetup( void ) {
    l2cap_unregister_service(PSM_HID_INTERRUPT);
    l2cap_unregister_service(PSM_HID_CONTROL);
}

void bt_kb_host_setup( void ) {
    bt_kb_resetup();
    key_hid_host_setup();
}

void bt_kb_device_setup( void ) {
    bt_kb_resetup();
    key_hid_device_setup();
}

void bt_kb_device_list_hci_connection( void ) {
    btstack_linked_list_iterator_t it;
    hci_connections_get_iterator(&it);
    while(btstack_linked_list_iterator_has_next(&it)){
        hci_connection_t * connection =
            (hci_connection_t *) btstack_linked_list_iterator_next(&it);
        printf( "%s: %s\n", __func__, bd_addr_to_str( connection->address ) );
    }
}

static bool bt_kb_get_paired_device_addr( int findId, bd_addr_t addr ) {
    link_key_t link_key;
    link_key_type_t type;
    btstack_link_key_iterator_t it;
    int ok = gap_link_key_iterator_init(&it);
    if (!ok) {
        log_error("could not initialize iterator");
        return false;
    }

    bool result = false;
    int id = -1;
    for ( ;gap_link_key_iterator_get_next(&it, addr, link_key, &type); id-- )
    {
        if ( findId == id ) {
            result = true;
            break;
        }
    }
    gap_link_key_iterator_done(&it);

    printf( "%s: %d, %d", __func__, findId, result );
    
    return result;
}

void bt_kb_list_paired_devices(void) {
    bd_addr_t  addr;
    link_key_t link_key;
    link_key_type_t type;
    btstack_link_key_iterator_t it;
    int ok = gap_link_key_iterator_init(&it);
    if (!ok) {
        log_error("could not initialize iterator");
        return;
    }

    printf( "%s:\n", __func__ );
    int id = -1;
    for ( ;gap_link_key_iterator_get_next(&it, addr, link_key, &type); id-- )
    {
        printf( " %d:   %s\n", id, bd_addr_to_str( addr ) );
    }
    gap_link_key_iterator_done(&it);        
    
}

void bt_kb_unpair( bd_addr_t addr ) {
    gap_drop_link_key_for_bd_addr( addr );
}

bool bt_kb_getAddr( int id, bd_addr_t addr )
{
    printf( "%s: %d\n", __func__, id );
    if ( id < 0 ) {
        return bt_kb_get_paired_device_addr( id, addr );
    }
    return my_gap_getAddr( id, addr );
}

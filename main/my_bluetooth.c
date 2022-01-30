#include <stdio.h>
#include "bluetooth.h"
#include "btstack.h"
#include "hid_keyboard_ctrl.h"
#include "btstack/my_gap_inquiry.h"
#include "my_console.h"
#include "my_keyboard.h"
#include "hog_key.h"
#include <ctype.h>
#include <le_device_db.h>

uint16_t bt_kb_getCod( void ) {
    return 0x2540;
}

void bt_kb_init( void ) {
}

void bt_kb_host_connect_to_device( bd_addr_t addr ) {
    printf( "%s: %s\n", __func__, bd_addr_to_str( addr ) );
    uint8_t status = key_hid_host_connect( addr );
    if (status != ERROR_CODE_SUCCESS){
        printf("HID host failed, status 0x%02x\n", status);
    }
}

void bt_kb_device_connect_to_host( bd_addr_t addr, bd_addr_type_t addrType ) {
    my_bt_info_t info;
    if ( console_get_bt_info( addr, &info ) ) {
        if ( ( info.type == bt_type_classic &&
               console_get_hid_device_mode() != hid_device_mode_bt ) ||
             ( info.type == bt_type_le &&
               console_get_hid_device_mode() != hid_device_mode_le ) )
        {
            printf( "%s: unmatch bluetooth mode\n", __func__ );
            return;
        }
    }

    printf( "%s: %s %d\n", __func__, bd_addr_to_str( addr ), addrType );
    if ( console_get_hid_device_mode() == hid_device_mode_bt ) {
        // pairing 済みの時の接続
        key_hid_device_connect( addr );
    }
}

void bt_kb_set_discoverable( bool flag ) {
    if ( console_get_hid_device_mode() == hid_device_mode_bt ) {
        if ( flag ) {
            key_hid_clear_device_target_addr();
        }
        gap_discoverable_control( flag ? 1 : 0 );
    } else {
        gap_advertisements_enable( flag ? 1 : 0 );
    }
}

void bt_kb_request_sendKey( void ) {
    if ( console_get_hid_device_mode() == hid_device_mode_bt ) {
        bt_hid_device_request_report();
    } else {
        hog_send_report();
    }
}

void bt_kb_set_sendKey( const char * pKeys ) {
    char oneChar = tolower( pKeys[0] );
    printf( "%s: %c(%s)\n", __func__, oneChar, pKeys );
    if ( oneChar >= 'a' && oneChar <= 'z' ) {
        oneChar = oneChar - 'a' + 0x4;
        my_kbd_add_key(0,oneChar);
        bt_kb_request_sendKey();
    }
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

static bool bt_kb_get_pairedInfo( bd_addr_t addr, bd_addr_type_t * pAddrType ) {
    my_bt_info_t info;
    if ( console_get_bt_info( addr, &info ) ) {
        memcpy( addr, info.pairAddr, sizeof( bd_addr_t ) );
        *pAddrType = info.pairAddrType;
        return true;
    }
    return false;
}

static bool bt_kb_get_paired_device_addr(
    int findId, bd_addr_t addr, bd_addr_type_t * pAddrType ) {
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
            result = bt_kb_get_pairedInfo( addr, pAddrType );
            break;
        }
    }
    gap_link_key_iterator_done(&it);

    if ( !result ) {
        int index = 0;
        for ( index = 0; index < le_device_db_max_count(); index++ ) {
            int addr_type;
            le_device_db_info( index, &addr_type, addr, NULL );
            if ( addr_type != BD_ADDR_TYPE_UNKNOWN ) {
                if ( id == findId ) {
                    result = bt_kb_get_pairedInfo( addr, pAddrType );
                    break;
                }
                id--;
            }
        }
    }

    
    printf( "%s: %d, %d", __func__, findId, result );
    
    return result;
}

static void bt_kb_dump_btInfoForAddr( int id, bd_addr_t addr ) {
    my_bt_info_t info;
    printf( " %d:   %s  ", id, bd_addr_to_str( addr ) );
    if ( console_get_bt_info( addr, &info ) ) {
        printf( " %s %d ", bd_addr_to_str( info.pairAddr ), info.pairAddrType );
        printf( "%s ", info.type == bt_type_classic ? "bt" : "le" );
        if ( info.role == bt_roleType_device ) {
            printf( "kybd  " );
        } else {
            printf( "host  " );
        }
        printf( "%s", info.name );
    }
    printf( "\n" );
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
        bt_kb_dump_btInfoForAddr( id, addr );
    }
    gap_link_key_iterator_done(&it);        

    int index = 0;
    for ( index = 0; index < le_device_db_max_count(); index++ ) {
        int addr_type;
        bd_addr_t addr;
        le_device_db_info( index, &addr_type, addr, NULL );
        if ( addr_type != BD_ADDR_TYPE_UNKNOWN ) {
            bt_kb_dump_btInfoForAddr( id, addr );
            id--;
        }
    }
}

void bt_kb_unpair( bd_addr_t addr ) {
    gap_drop_link_key_for_bd_addr( addr );

    int index = 0;
    for ( index = 0; index < le_device_db_max_count(); index++ ) {
        int addr_type;
        bd_addr_t work_addr;
        le_device_db_info( index, &addr_type, work_addr, NULL );
        if ( addr_type != BD_ADDR_TYPE_UNKNOWN &&
             memcmp( work_addr, addr, sizeof( bd_addr_t ) ) == 0 )
        {
            le_device_db_remove( index );
#ifdef ENABLE_LE_PRIVACY_ADDRESS_RESOLUTION
            hci_remove_le_device_db_entry_from_resolving_list(i);
#endif
        }
    }
}

bool bt_kb_getPairAddr( int id, bd_addr_t addr, bd_addr_type_t * pAddrType )
{
    printf( "%s: %d\n", __func__, id );
    if ( id < 0 ) {
        return bt_kb_get_paired_device_addr( id, addr, pAddrType );
    }
    *pAddrType = BD_ADDR_TYPE_LE_PUBLIC;
    return my_gap_getAddr( id, addr );
}

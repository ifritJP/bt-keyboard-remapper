#include <stdio.h>
#include "bluetooth.h"
#include "btstack.h"
#include "hid_key.h"


static const char * device_addr_string = "EC:00:FE:00:09:56";

void bt_kb_host_connect( void ) {
    bd_addr_t addr;
    sscanf_bd_addr( device_addr_string, addr );
    uint8_t status = key_hid_host_connect( addr );
    if (status != ERROR_CODE_SUCCESS){
        printf("HID host failed, status 0x%02x\n", status);
    }
}

void bt_kb_device_connect( void ) {
    bd_addr_t addr;
    // 接続先ホストの Bluetooth Address を指定する
    sscanf_bd_addr( "00:1B:DC:06:61:EB", addr );
    printf( "hid_device_connect\n" );
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

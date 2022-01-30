#include <bluetooth.h>

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

  extern uint16_t bt_kb_getCod( void );
  extern const char * bt_kb_getDeviceName( void );
  extern void bt_kb_host_connect_to_device( bd_addr_t addr );
  extern void bt_kb_device_connect_to_host( bd_addr_t addr, bd_addr_type_t addrType );
  extern void bt_kb_set_discoverable( bool flag );
  extern void bt_kb_request_sendKey( void );
  extern void bt_kb_set_sendKey( const char * pKeys );
  extern void bt_kb_host_setup( void );
  extern void bt_kb_device_setup( void );
  extern void bt_kb_init( void );
  extern void bt_kb_device_list_hci_connection( void );
  extern void bt_kb_list_paired_devices(void);
  extern void bt_kb_unpair( bd_addr_t addr );
  extern bool bt_kb_getPairAddr( int id, bd_addr_t addr, bd_addr_type_t * pAddrType );

#ifdef __cplusplus
}
#endif

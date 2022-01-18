#include "btstack.h"

typedef enum {
    hid_device_mode_bt,
    hid_device_mode_le,
} hid_device_mode_t;


extern uint8_t key_hid_host_connect( bd_addr_t addr );
extern void key_hid_device_connect( bd_addr_t addr );
extern void console_dump_hid_channl_info( void );
extern void hid_embedded_start_typing( const char * pTxt );
extern void key_hid_host_setup(void);
extern void key_hid_device_setup( void );
extern void key_hid_clear_device_target_addr( void );
extern void key_hid_set_device_mode( hid_device_mode_t mode );

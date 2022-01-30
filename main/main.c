#include "btstack_port_esp32.h"
#include "btstack_run_loop.h"
#include "hci_dump.h"
#include "hci_dump_embedded_stdout.h"
#include "nvs.h"
#include "nvs_flash.h"

#include <stddef.h>

#include "my_ota.h"
#include "cmd_wifi.h"
#include "my_console.h"
#include "stateCtrl.h"
#include "my_bluetooth.h"
#include "my_keyboard.h"
#include "btstack/my_gap_inquiry.h"


extern int btstack_main(int argc, const char * argv[]);

static void stdout_log_packet(
    uint8_t packet_type, uint8_t in, uint8_t *packet, uint16_t len) {
}

static void stdout_log_message(const char * format, va_list argptr){
}

#ifdef __AVR__
void stdout_log_message_P(
    int log_level, PGM_P * format, va_list argptr){
}
#endif

static void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

int app_main(void)
{
    initialize_nvs();
    console_loadConfig();

    my_kbd_init();

    
    extern void arduinoSetup(void);
    arduinoSetup();

    /* const hci_dump_t * hci_dump_impl = hci_dump_posix_fs_get_instance(); */
    /* hci_dump_init(hci_dump_impl); */
    
    // optional: enable packet logger
    //hci_dump_init(hci_dump_embedded_stdout_get_instance());
    hci_dump_t hci_dump_instance = *hci_dump_embedded_stdout_get_instance();
    hci_dump_instance.log_packet = stdout_log_packet;
    //hci_dump_instance.log_message = stdout_log_message;
#ifdef __AVR__
    hci_dump_instance.log_message_P = stdout_log_message_P;
#endif

    hci_dump_init( &hci_dump_instance );

    // Configure BTstack for ESP32 VHCI Controller
    btstack_init();

    // Setup example
    btstack_main(0, NULL);
  
    

    bt_kb_init();

    my_gap_setup();
    
    extern void console_app_main(void);
    console_app_main();

    wifi_setting_t setting;
    if ( console_get_wifi_setting( &setting ) ) {
        wrap_wifi_join( setting.ssid, setting.pass, 2000 );
        start_otad( setting.otaAuthB64 );
    }

    state_run();
    
    // Enter run loop (forever)
    btstack_run_loop_execute();

    return 0;
}

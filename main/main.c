/*
 * Copyright (C) 2020 BlueKitchen GmbH
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
 */

/*
 *  main.c
 *
 *  Minimal main application that initializes BTstack, prepares the example and enters BTstack's Run Loop.
 *
 *  If needed, you can create other threads. Please note that BTstack's API is not thread-safe and can only be
 *  called from BTstack timers or in response to its callbacks, e.g. packet handlers.
 */

#include "btstack_port_esp32.h"
#include "btstack_run_loop.h"
#include "hci_dump.h"
#include "hci_dump_embedded_stdout.h"

#include <stddef.h>

#include "my_ota.h"
#include "cmd_wifi.h"
#include "my_console.h"
#include "stateCtrl.h"

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

int app_main(void)
{
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

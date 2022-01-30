#include "arduinoWrap.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <my_bluetooth.h>

#include <esp_log.h>
#include "my_console.h"
#include "esp_timer.h"

#define INTERVAL_MILISEC 50
#define TAG "STATE"

typedef enum {
    /** setup mode */
    ui_state_kind_setup,
    
    /** 起動中 */
    ui_state_kind_booting,
    /** BT ホストへ接続中 */
    ui_state_kind_connect_to_host,
    /** 待ち */
    ui_state_kind_dummy_wait,
    /** BT デバイスへ接続中 */
    ui_state_kind_connect_from_device,
    /** キー変換処理。通常状態。 */
    ui_state_kind_remap_key,
    /** メニュー選択中 */
    ui_state_kind_select_menu,
    /** 値選択中 */
    ui_state_kind_select_value,
    
} ui_state_kind_t;

typedef enum {
    ui_menu_bt_device_mode,


    
    number_of_ui_menu,
} ui_menu_kind_t;

typedef struct {
    ui_state_kind_t kind;
    ui_menu_kind_t menuKind;

    volatile bool connectedToHost;
    volatile bool connectedFromDevice;
    int64_t prevTime;
    
} state_info_t;


typedef enum {
    ui_led_mode_off,
    ui_led_mode_on,
    ui_led_mode_blink,
    ui_led_mode_blink_long,
    ui_led_mode_lazy,
    ui_led_mode_emergency,

    last_of_ui_led_mode = ui_led_mode_emergency
} ui_led_mode_t;

typedef struct {
    int subStep;
    ui_led_mode_t ledMode;
    uint32_t ledColor;
} state_led_info_t;

typedef struct {
    bool prevPressed;
    bool prevPressedFix;
    int keptCount;
    int countForLongPress;
    bool longPressed;
    bool edgeLongPressed;
    int countForLongLongPress;
    bool longLongPressed;
    bool edgeLongLongPressed;
    bool clicked;
} state_button_info_t;

#define LED_SUBSTEP_MAX 12

static const uint8_t s_ledBlinkPattrn[][LED_SUBSTEP_MAX] = {
    // ui_led_mode_off,
    {},
    // ui_led_mode_on,
    { 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1 },
    // ui_led_mode_blink,
    { 0x1, 0x1, 0x0, 0x0, 0x1, 0x1, 0x0, 0x0, 0x1, 0x1, 0x0, 0x0 },
    // ui_led_mode_blink_long,
    { 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0 },
    // ui_led_mode_lazy,
    { 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0, 0x0 },
    // ui_led_mode_emergency,
    { 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0, 0x1, 0x0 },
};

static const uint32_t s_ledRgbStep[] = {
    0xff0000,
    0x00ff00,
    0x0000ff,
    0x904000,
    0x009040,
    0x400090,
};

static void state_led_process( state_led_info_t * pInfo ) {

    static uint32_t prevBlinkPattern = 0;
    static uint32_t prevColor = 0;
    
    pInfo->subStep++;
    if ( pInfo->subStep >= LED_SUBSTEP_MAX ) {
        pInfo->subStep = 0;
    }
    
    uint32_t onLed = s_ledBlinkPattrn[ pInfo->ledMode ][ pInfo->subStep ];
    if ( prevBlinkPattern != onLed || prevColor != pInfo->ledColor ) {
        if ( onLed == 0 ) {
            ui_led_set( 0 );
        } else {
            ui_led_set( pInfo->ledColor );
        }
    }
    prevBlinkPattern = onLed;
    prevColor = pInfo->ledColor;
}

static void state_button_process( state_button_info_t * pInfo ) {
    bool isPressed = ui_button_isPressed();
    pInfo->clicked = false;
    pInfo->edgeLongPressed = false;
    pInfo->edgeLongLongPressed = false;
    if ( pInfo->prevPressed == isPressed ) {
        if ( pInfo->keptCount >= 1 ) {
            if ( pInfo->prevPressedFix != isPressed ) {
                pInfo->prevPressedFix = isPressed;
                printf( "fix %d\n", isPressed );
                if ( !isPressed ) {
                    if ( !pInfo->longPressed ) {
                        pInfo->clicked = true;
                        printf( "clicked\n" );
                    } else {
                        pInfo->longPressed = false;
                        pInfo->longLongPressed = false;
                    }
                }
            }
            if ( pInfo->keptCount >= pInfo->countForLongPress ) {
                if ( isPressed ) {
                    if ( pInfo->keptCount >= pInfo->countForLongLongPress ) {
                        if ( !pInfo->longLongPressed ) {
                            pInfo->longLongPressed = true;
                            pInfo->edgeLongLongPressed = true;
                            printf( "long long pressed\n" );
                        }
                    } else {
                        pInfo->keptCount++;
                        if ( !pInfo->longPressed ) {
                            pInfo->longPressed = true;
                            pInfo->edgeLongPressed = true;
                            printf( "long pressed\n" );
                        }
                    }
                }
            } else {
                pInfo->keptCount++;
            }
        } else {
            pInfo->keptCount++;
        }
    } else {
        pInfo->prevPressed = isPressed;
        pInfo->keptCount = 1;
    }
}


static void state_switchState(
    state_info_t * pInfo, ui_state_kind_t newKind,
    state_led_info_t * pLedInfo, my_config_t * pConfig )
{
    ui_state_kind_t prevKind = pInfo->kind;

    ESP_LOGI( TAG, "%s: prevKind (%d) -> newKind (%d)", __func__, prevKind, newKind );
    
    switch ( prevKind ) {
    case ui_state_kind_booting:
        break;
    case ui_state_kind_connect_to_host:
        break;
    case ui_state_kind_dummy_wait:
        break;
    case ui_state_kind_connect_from_device:
        break;
    case ui_state_kind_remap_key:
        break;
    case ui_state_kind_select_menu:
        break;
    case ui_state_kind_select_value:
        break;
    default:
        break;
    }

    pInfo->kind = newKind;
    switch ( newKind ) {
    case ui_state_kind_booting:
        break;
    case ui_state_kind_connect_to_host:
        pLedInfo->ledMode = ui_led_mode_emergency;
        pLedInfo->ledColor = 0x00ff00;
        bt_kb_device_setup();
        ESP_LOGI( TAG, "%s: called bt_kb_device_setup", __func__ );
        // BT 初期化が終わるまで、とりあえず 1 秒待つ
        vTaskDelay( 1000 * 2 / portTICK_PERIOD_MS );
        my_bt_info_t info;
        if ( console_get_bt_info( pConfig->toHostAddr, &info ) ) {
            bt_kb_device_connect_to_host( info.pairAddr, info.pairAddrType );
        }
        break;
    case ui_state_kind_dummy_wait:
        pInfo->prevTime = esp_timer_get_time() / 1000;
        break;
    case ui_state_kind_connect_from_device:
        pLedInfo->ledMode = ui_led_mode_blink_long;
        bt_kb_host_setup();
        // bt_kb_host_connect_to_device();
        break;
    case ui_state_kind_remap_key:
        pLedInfo->ledMode = ui_led_mode_on;
        pLedInfo->ledColor = 0x00ff00;
        break;
    case ui_state_kind_select_menu:
        break;
    case ui_state_kind_select_value:
        break;
    default:
        break;
    }
}

static state_info_t s_stateIno = {
    .kind = ui_state_kind_booting,
};

static void state_process(
    state_info_t * pStateInfo,
    state_led_info_t * pLedInfo, my_config_t * pConfig )
{
    switch ( pStateInfo->kind ) {
    case ui_state_kind_setup:
        break;
    case ui_state_kind_booting:
        state_switchState(
            pStateInfo, ui_state_kind_connect_to_host, pLedInfo, pConfig );
        break;
    case ui_state_kind_connect_to_host:
        if ( pStateInfo->connectedToHost ) {
            // 接続した後、直ぐに次の処理を続けると正常に動作しないので、
            // とりあえずダミーの待ち処理を入れる。
            state_switchState(
                pStateInfo, ui_state_kind_dummy_wait, pLedInfo, pConfig );
        }
        break;
    case ui_state_kind_dummy_wait:
        {
            int64_t now = esp_timer_get_time() / 1000;
            if ( now - pStateInfo->prevTime > 1000 * 3 ) {
                state_switchState(
                    pStateInfo, ui_state_kind_connect_from_device, pLedInfo, pConfig );
            }
        }
        break;
    case ui_state_kind_connect_from_device:
        if ( pStateInfo->connectedFromDevice ) {
            state_switchState(
                pStateInfo, ui_state_kind_remap_key, pLedInfo, pConfig );
        }
        break;
    case ui_state_kind_remap_key:
        break;
    case ui_state_kind_select_menu:
        break;
    case ui_state_kind_select_value:
        break;
    default:
        break;
    }
}


static void state_task(void * pParam)
{
    state_led_info_t ledInfo = {
        .subStep = 0,
        .ledMode = ui_led_mode_lazy,
        .ledColor = 0xff0000,
    };
    state_button_info_t buttonInfo = {
        .countForLongPress = 6,
        .countForLongLongPress = 85,
    };

    my_config_t * pConfig = console_get_config();

    if ( pConfig->mode == my_config_mode_setup ) {
        s_stateIno.kind = ui_state_kind_setup;
    } else {
        s_stateIno.kind = ui_state_kind_booting;
        ledInfo.ledMode = ui_led_mode_blink_long;
    }

    ui_update();
    if ( ui_button_isPressed() ) {
        vTaskDelay( 1000 / portTICK_PERIOD_MS );
        ui_update();
        if ( ui_button_isPressed() ) {
            s_stateIno.kind = ui_state_kind_setup;
            ledInfo.ledMode = ui_led_mode_emergency;
        }
    }

    
    int step = 0;
    
    while ( true ) {
        vTaskDelay( INTERVAL_MILISEC / portTICK_PERIOD_MS );

        ui_update();
        state_led_process( &ledInfo );
        state_button_process( &buttonInfo );


        state_process( &s_stateIno, &ledInfo, pConfig );
        
        

        if ( buttonInfo.clicked ) {
            step = (step + 1) % (sizeof( s_ledRgbStep ) / sizeof( s_ledRgbStep[0] ));
            ledInfo.ledColor = s_ledRgbStep[ step ];
        }
        if ( buttonInfo.edgeLongPressed ) {
            ledInfo.ledMode =
                (ui_led_mode_t)((ledInfo.ledMode + 1) % (last_of_ui_led_mode+1));
        }
    }
}

void state_run( void ) {
    xTaskCreate( state_task, "state_task", 4 * 1024, NULL,
		 configMAX_PRIORITIES - 3, NULL );
}

int state_dump( void )
{
    printf( "state:\n" );
    printf( "  kind = %d\n", s_stateIno.kind );
    printf( "  menuKind = %d\n", s_stateIno.menuKind );
    printf( "  connectedToHost = %d\n", s_stateIno.connectedToHost );
    printf( "  connectedFromDevice = %d\n", s_stateIno.connectedFromDevice );
    
    return 0;
}


void state_connectedToHost( bool flag ) {
    s_stateIno.connectedToHost = flag;
}

void state_connectedFromDevice( bool flag ) {
    s_stateIno.connectedFromDevice = flag;
}


#include "arduinoWrap.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define INTERVAL_MILISEC 50

typedef enum {
    ui_led_mode_off,
    ui_led_mode_on,
    ui_led_mode_blink,
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
    bool clicked;
} state_button_info_t;


#define LED_SUBSTEP_MAX 12

static const uint8_t s_ledBlinkPattrn[][LED_SUBSTEP_MAX] = {
    {},
    { 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1 },
    { 0x1, 0x1, 0x0, 0x0, 0x1, 0x1, 0x0, 0x0, 0x1, 0x1, 0x0, 0x0 },
    { 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0, 0x0 },
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
                    }
                }
            }
            if ( pInfo->keptCount >= pInfo->countForLongPress ) {
                if ( isPressed ) {
                    if ( !pInfo->longPressed ) {
                        pInfo->longPressed = true;
                        pInfo->edgeLongPressed = true;
                        printf( "long pressed\n" );
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




static void state_task(void * pParam)
{
    state_led_info_t ledInfo = {
        .subStep = 0,
        .ledMode = ui_led_mode_lazy,
        .ledColor = 0xff0000,
    };
    state_button_info_t buttonInfo = {
        .countForLongPress = 6,
    };

    int step = 0;
    
    while ( true ) {
        vTaskDelay( INTERVAL_MILISEC / portTICK_PERIOD_MS );

        ui_update();
        state_led_process( &ledInfo );
        state_button_process( &buttonInfo );

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

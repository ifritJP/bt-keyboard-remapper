#include "Arduino.h"
#include "M5Atom.h"

static volatile int sleepMiliSec = 2000;

static void led_task(void * pParam)
{
  const int rgb[] = {
    0x000000,
    0x0000ff,
    0xff0000,
    0xff00ff,
    0x00ff00,
    0xffff00,
    0x00ffff,
    0xffffff,
  };
  while (1) {
    int count;
    for ( count = 0; count < sizeof( rgb ) / sizeof( rgb[0] ); count++ ) {
      M5.dis.drawpix(0, rgb[ count ] );
      vTaskDelay(sleepMiliSec / portTICK_PERIOD_MS);
    }
  }
}

static void buttonTask( void * pParam )
{
  while (1) {
    M5.update();

    if (M5.Btn.wasPressed()) {
      printf( "pressed\n" );
      if ( sleepMiliSec == 2000 ) {
	sleepMiliSec = 500;
      } else {
	sleepMiliSec = 2000;
      }
      vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
    vTaskDelay( 100 / portTICK_PERIOD_MS );
  }
}

extern "C" void arduinoSetup(void) {
  //// initArduino() 実行すると正常に動作しなかったのでコメントアウト
  // initArduino();

  M5.begin(false, false, true);

  void * pParam = NULL;
  xTaskCreate( led_task, "led_task", 2 * 1024, NULL,
	       configMAX_PRIORITIES - 3, &pParam );

  xTaskCreate( buttonTask, "buttonTask", 1 * 1024, NULL,
	       configMAX_PRIORITIES - 3, &pParam );
}

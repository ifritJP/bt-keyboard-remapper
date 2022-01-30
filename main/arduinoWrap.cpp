#include "Arduino.h"
#include "M5Atom.h"

#include "arduinoWrap.h"

extern "C" {
  void ui_led_set( uint32_t rgb ) {
      M5.dis.drawpix(0, rgb );
  }

  bool ui_button_isPressed( void ) {
    return M5.Btn.isPressed();
  }

  void ui_update( void ) {
    M5.update();
  }
  
  void arduinoSetup(void) {
    //// initArduino() 実行すると正常に動作しなかったのでコメントアウト
    // initArduino();

    M5.begin(false, false, true);
  }
}

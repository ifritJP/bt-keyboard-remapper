#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
  extern void ui_led_set( uint32_t rgb );
  extern bool ui_button_isPressed( void );
  extern void arduinoSetup(void);
  extern void ui_update( void );

#ifdef __cplusplus
}
#endif

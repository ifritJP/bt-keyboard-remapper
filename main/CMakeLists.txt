cmake_minimum_required(VERSION 3.5)

idf_component_register(
  SRCS main.c hid_keyboard_demo.c hog_keyboard_demo.c
  console_example_main.c
  my_ota.c
  cmd_wifi.c
  my_console.c
  my_bluetooth.c
  #  console.c
  mutex.c
  arduinoWrap.cpp
  M5Atom/src/M5Atom.cpp
  M5Atom/src/utility/Button.cpp
  M5Atom/src/utility/LED_DisPlay.cpp
  M5Atom/src/utility/M5Timer.cpp
  M5Atom/src/utility/MPU6886.cpp
  M5Atom/src/utility/MahonyAHRS.cpp
  M5Atom/src/utility/Task.cpp
  FastLED/src/bitswap.cpp
  FastLED/src/colorpalettes.cpp
  FastLED/src/colorutils.cpp
  FastLED/src/FastLED.cpp
  FastLED/src/hsv2rgb.cpp
  FastLED/src/lib8tion.cpp
  FastLED/src/noise.cpp
  FastLED/src/platforms/esp/32/clockless_rmt_esp32.cpp
  FastLED/src/platforms.cpp
  FastLED/src/power_mgt.cpp
  FastLED/src/wiring.cpp


  
  INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
  .
  M5Atom/src
  M5Atom/src/utility
  FastLED/src
  )

target_compile_options(${COMPONENT_TARGET} PUBLIC
  -Wno-unused-but-set-parameter
  -DESP32)


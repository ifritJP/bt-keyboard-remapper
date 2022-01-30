cmake_minimum_required(VERSION 3.5)

idf_component_register(
  SRCS
  main.c
  hid_reportDesc.c
  HID.c
  convMap.c
  my_keyboard.c
  Config.c
  stateCtrl.c
  my_ota.c
  my_console.c
  my_bluetooth.c
  hid_keyboard_ctrl.c
  hog_keyboard_ctrl.c
  depends/btstack/gap_inquiry.c
  depends/btstack/console_example_main.c
  depends/btstack/cmd_wifi.c
  #  console.c
  mutex.c
  arduinoWrap.cpp
  depends/M5Atom/src/M5Atom.cpp
  depends/M5Atom/src/utility/Button.cpp
  depends/M5Atom/src/utility/LED_DisPlay.cpp
  depends/M5Atom/src/utility/M5Timer.cpp
  depends/M5Atom/src/utility/MPU6886.cpp
  depends/M5Atom/src/utility/MahonyAHRS.cpp
  depends/M5Atom/src/utility/Task.cpp
  depends/FastLED/src/bitswap.cpp
  depends/FastLED/src/colorpalettes.cpp
  depends/FastLED/src/colorutils.cpp
  depends/FastLED/src/FastLED.cpp
  depends/FastLED/src/hsv2rgb.cpp
  depends/FastLED/src/lib8tion.cpp
  depends/FastLED/src/noise.cpp
  depends/FastLED/src/platforms/esp/32/clockless_rmt_esp32.cpp
  depends/FastLED/src/platforms.cpp
  depends/FastLED/src/power_mgt.cpp
  depends/FastLED/src/wiring.cpp


  
  INCLUDE_DIRS "${CMAKE_CURRENT_BINARY_DIR}"
  .
  depends
  depends/btstack
  depends/M5Atom/src
  depends/M5Atom/src/utility
  depends/FastLED/src
  )

target_compile_options(${COMPONENT_TARGET} PUBLIC
  -Wno-unused-but-set-parameter
  -DESP32)


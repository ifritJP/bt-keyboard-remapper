#pragma once

#include "HID.h"
#include "convMap.h"


extern bool ReadRemap(
  const void * pBuf, int size,
  Code2HidCode_t * pConv, HIDKeyboard_t * pKeyboard );
extern int WriteRemap(
  void * pBuf, int size,
  const Code2HidCode_t * pConv, const HIDKeyboard_t * pKeyboard );

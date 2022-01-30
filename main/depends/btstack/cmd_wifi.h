/* Console example — declarations of command registration functions.

   modified 2022 ifritJP   
   
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Register WiFi functions
void register_wifi(void);
  extern bool wrap_wifi_join(const char *ssid, const char *pass, int timeout_ms);

#ifdef __cplusplus
}
#endif

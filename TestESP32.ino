/**
 * @brief 
 * @file TestESP32.ino
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */

#include "Config.h"
#include "App_Leds.h"
#include "App_Wifi.h"

uint32_t u32Timeout = 0;
uint8_t u8FlipFlop = 0;

void setup() {
#if APP_PRINT
    Serial.begin(115200);
#endif
    pinMode(MODULE_LED, OUTPUT);
#if APP_FASTLED
    AppLED_init();
#endif
#ifdef APP_WIFI
#if APP_PRINT
    Serial.println("Connection to " MY_SSID);
#endif
    eAppWifi_init(5000);
#endif
}

void loop() {
#ifdef APP_WIFI
    eAppWifi_Tasking();
#endif
#if APP_FASTLED
    AppLED_showLoop();
#endif
}

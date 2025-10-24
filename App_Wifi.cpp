#include "App_Wifi.h"
#include "App_Leds.h"

#if defined(APP_WIFI) && APP_WIFI

/**
 * @brief Initialize WiFi
 * 
 * @param u32Timeout 
 * @return eApp_RetVal 
 */
eApp_RetVal eAppWifi_init(uint32_t u32Timeout) {
    eApp_RetVal eRet = eRet_Ok;
    uint32_t u32ExitTimeout = millis() + u32Timeout;
    WiFi.setHostname(MY_DEVICENAME);
    if (WiFi.begin(MY_SSID, MY_PWD) != WL_CONNECTED) {
        while ((u32ExitTimeout > millis()) && (WiFi.status() != WL_CONNECTED)) {
            delay(500);
#if APP_PRINT
            Serial.print(". ");
#endif
        }
    }
    eRet = (WiFi.status() == WL_CONNECTED) ? eRet_Ok : eRet_Error;
    return eRet;
}

void eAppWifi_RssiGauge(void) {
    if (WiFi.status() == WL_CONNECTED) {
        int16_t i16Rssi = WiFi.RSSI();
        AppLED_fillGauge(i16Rssi, -100, -60);
#if APP_PRINT
        Serial.printf("[" MY_SSID "] RSSI: %d\r\n", i16Rssi);
#endif
        AppLED_blink(0);
    }
    else {
        AppLED_blink(1);
    }
}

void eAppWifi_Tasking(void) {
    static uint32_t u32Timeout = 0;
    if (u32Timeout < millis()) {
        u32Timeout = WIFI_TASKING + millis();
        // eAppWifi_RssiGauge();
    }
}

#endif // APP_WIFI

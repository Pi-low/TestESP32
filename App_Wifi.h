/**
 * @brief Config wifi application
 * @file App_Wifi.h
 * @version 0.1
 * @date 2025-10-14
 * @author Nello
 */

#ifndef _APP_WIFI_H
#define _APP_WIFI_H

#include "Config.h"

#if defined(APP_WIFI) && APP_WIFI

#define WIFI_TASKING 500
#define MY_DEVICENAME   "Proto01"
// #define MY_SSID         ""
// #define MY_PWD          ""

#include <WiFi.h>

eApp_RetVal eAppWifi_init(uint32_t u32Timeout);
void eAppWifi_RssiGauge(void);
void eAppWifi_Tasking(void);

#endif // APP_WIFI

#endif // _APP_WIFI_H

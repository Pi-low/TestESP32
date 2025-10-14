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

#if defined(MY_SSID) && defined (MY_PWD)
#define APP_WIFI
#endif

#ifdef APP_WIFI

#define WIFI_TASKING 500

#include <WiFi.h>

eApp_RetVal eAppWifi_init(uint32_t u32Timeout);
void eAppWifi_RssiGauge(void);
void eAppWifi_Tasking(void);

#endif // APP_WIFI

#endif // _APP_WIFI_H

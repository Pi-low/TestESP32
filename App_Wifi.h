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
#include "App_PrintUtils.h"

#if defined(APP_WIFI) && APP_WIFI
#include <WiFi.h>

#define WIFI_TASKING 1000

eApp_RetVal eAppWifi_init(void);
bool bAppWifi_SyncWifiConfig(void);

#endif // APP_WIFI

#endif // _APP_WIFI_H

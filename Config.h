/**
 * @brief Application configuration
 * @file Config.h
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include <Arduino.h>
#include "Typedefs.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

/*******************************************************************************
 *  GENERAL CONFIGURATION 
 ******************************************************************************/
#define APP_TASKS       1 // use FreeRTOS
#define APP_WIFI        1 // actiavate wifi tasking
#define APP_FASTLED     1 // activate ledstrip management
#define ESP_LED_PIN     8
#define APP_PRINT       1

#define CONFIG_FILE_PATH    "/config.cfg"

/*******************************************************************************
 *  DEBUG CONFIGURATION 
 ******************************************************************************/

/*******************************************************************************
 *  FREERTOS
 ******************************************************************************/
#if APP_TASKS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#endif

extern JsonDocument jAppCfg_Config;

eApp_RetVal eAppConfig_init(void);
eApp_RetVal eAppCfg_LoadConfig(const char *pcFromFilePath);
eApp_RetVal eAppCfg_SaveConfig(const char *pcToFilePath);
eApp_RetVal eAppCfg_SetDefaultConfig(void);
eApp_RetVal eAppCfg_ResetParamKey(const char* pcObjectKey);
bool bAppCfg_LockJson(void);
bool bAppCfg_UnlockJson(void);

#endif // _CONFIG_H

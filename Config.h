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

/*******************************************************************************
 *  GENERAL CONFIGURATION 
 ******************************************************************************/
#define APP_TASKS       1 // use FreeRTOS
#define APP_WIFI        0 // actiavate wifi tasking
#define APP_FASTLED     1 // activate ledstrip management
#define ESP_LED_PIN     8
#define APP_PRINT       1

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
#define APP_TRACE(x)     vAppPrintUtils_Print(x, strlen(x))
#endif

eApp_RetVal eAppConfig_init(void);

#endif // _CONFIG_H

/**
 * @brief Application configuration
 * @file Config.h
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */

#ifndef _CONFIG_H
#define _CONFIG_H

#include "Typedefs.h"

/*******************************************************************************
 *  GENERAL CONFIGURATION 
 ******************************************************************************/
#define APP_TASKS       1 // use FreeRTOS
#define APP_WIFI        0 // actiavate wifi tasking
#define APP_FASTLED     1 // activate ledstrip management
#define ESP_LED_PIN     8
#define APP_PRINT       1

#if APP_TASKS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Defines main task
#define MAIN_TASK            "APP_MAIN"
#define MAIN_TASK_HEAP      512
#define MAIN_TASK_PARAM     NULL
#define MAIN_TASK_PRIO      1
#define MAIN_TASK_HANDLE    NULL
#define MAIN_TASK_CYCLE     500
#endif

#endif // _CONFIG_H

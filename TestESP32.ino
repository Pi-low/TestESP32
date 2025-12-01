/**
 * @brief 
 * @file TestESP32.ino
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */

#include "Config.h"
#include "App_PrintUtils.h"
#include "App_Leds.h"
#include "App_Wifi.h"
#include "App_Cli.h"

#if APP_TASKS

/******************************************************************************/
/* CONFIGURATION                                                              */
/******************************************************************************/
#define MAIN_TASK           "APP_MAIN"
#define MAIN_TASK_HEAP      configMINIMAL_STACK_SIZE
#define MAIN_TASK_PARAM     NULL
#define MAIN_TASK_PRIO      1
#define MAIN_TASK_HANDLE    NULL
#define MAIN_TASK_CYCLE     500

void vAppMain(void *pvParam);
#endif

void setup() {
#if APP_PRINT
    Serial.begin(115200);
    vAppPrintUtils_init();
    eAppConfig_init();
    vAppCli_init();
#endif
    pinMode(ESP_LED_PIN, OUTPUT);
#if APP_FASTLED
    AppLED_init();
#endif
#if APP_WIFI
#if APP_PRINT
    Serial.println("Connection to " MY_SSID);
#endif
    eAppWifi_init(5000);
#endif
#if APP_TASKS
    xTaskCreate(vAppMain, MAIN_TASK, MAIN_TASK_HEAP, MAIN_TASK_PARAM, MAIN_TASK_PRIO, MAIN_TASK_HANDLE);
#endif
}

void loop() {
#if APP_WIFI
    eAppWifi_Tasking();
#endif
#if (APP_FASTLED) && (!APP_TASKS)
    AppLED_showLoop();
#endif
}

#if APP_TASKS
void vAppMain(void *pvParam) {
    bool bToggle = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(MAIN_TASK_CYCLE));
        bToggle ^= 1;
        digitalWrite(ESP_LED_PIN, bToggle);
    }
}
#endif

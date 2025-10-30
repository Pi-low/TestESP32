/**
 * @brief Thread safe printing management
 * @file App_PrintUtils.cpp
 * @version 0.1
 * @date 2025-10-30
 * @author Nello
 */

#include "App_PrintUtils.h"

#if defined(APP_TASKS) && APP_TASKS

#define PRINT_UTILS_TASK    "APP_PRINT"
#define PRINT_UTILS_HEAP    1024
#define PRINT_UTILS_PARAM   NULL
#define PRINT_UTILS_PRIO    2
#define PRINT_UTILS_HANDLE  NULL
#define PRINT_UTILS_PERIOD  100

#define _PRINTQ_SIZE        16

static QueueHandle_t serialPrintQ;

void vAppPrintUtils_Task(void* pvArg);

/*******************************************************************************
 * @brief Initialize printing utils
 * 
 ******************************************************************************/
void vAppPrintUtils_init(void) {
    serialPrintQ = xQueueCreate(_PRINTQ_SIZE, PRINT_UTILS_MAX_BUF * sizeof(char));
    if (serialPrintQ == NULL) {
        Serial.println("[App_PrintUtils] Cannot create queue !");
    }
    else {
        xTaskCreate(vAppPrintUtils_Task, PRINT_UTILS_TASK, PRINT_UTILS_HEAP, PRINT_UTILS_PARAM, PRINT_UTILS_PRIO, PRINT_UTILS_HANDLE);
    }
}

/*******************************************************************************
 * @brief Print data to serial
 * 
 ******************************************************************************/
void vAppPrintUtils_Print(const char* pcDataToPrint, BaseType_t xLength) {
    if (serialPrintQ != NULL) {
        char pcbuffer[128] = {0};
        memcpy(pcbuffer, pcDataToPrint, MIN(xLength, PRINT_UTILS_MAX_BUF-1));
        xQueueSend(serialPrintQ, pcbuffer, portMAX_DELAY);
    }
    else {
        Serial.print(pcDataToPrint);
    }
}

/*******************************************************************************
 * @brief Print task
 * 
 ******************************************************************************/
void vAppPrintUtils_Task(void* pvArg) {
    char pcBuffer[PRINT_UTILS_MAX_BUF];
    TickType_t xLastWakeTime = xTaskGetTickCount();
    while (1) {
        // vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(PRINT_UTILS_PERIOD));
        if (xQueueReceive(serialPrintQ, pcBuffer, portMAX_DELAY) == pdPASS) {
            Serial.print(pcBuffer);
        }
    }
}

#endif

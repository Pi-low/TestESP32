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
#define PRINT_UTILS_HEAP    (configMINIMAL_STACK_SIZE*2)
#define PRINT_UTILS_PARAM   NULL
#define PRINT_UTILS_PRIO    2
#define PRINT_UTILS_HANDLE  NULL
#define PRINT_UTILS_PERIOD  100

#define _PRINTQ_SIZE        64

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
        uint8_t u8SecureLoop = 0;
        char *pcIndex = (char*) pcDataToPrint;
        BaseType_t xLocalLength = xLength;
        BaseType_t xDataToSend = 0;
        char pcbuffer[PRINT_UTILS_MAX_BUF];
        do
        {
            memset(pcbuffer, 0, PRINT_UTILS_MAX_BUF);
            xDataToSend = (xLocalLength > (PRINT_UTILS_MAX_BUF-1)) ? (PRINT_UTILS_MAX_BUF-1) : xLocalLength;
            xLocalLength = ((xLocalLength - xDataToSend) > 0) ? xLocalLength - xDataToSend : 0;
            memcpy(pcbuffer, pcIndex, xDataToSend);
            xQueueSend(serialPrintQ, pcbuffer, portMAX_DELAY);

            pcIndex += xDataToSend;
            u8SecureLoop++;
        } while ((xLocalLength > 0) && (pcIndex != NULL) && (u8SecureLoop < _PRINTQ_SIZE));

        if (u8SecureLoop >= _PRINTQ_SIZE)
        {
            snprintf(pcbuffer, PRINT_UTILS_MAX_BUF, "Warning: end of print (%d)\r\n", u8SecureLoop);
            xQueueSend(serialPrintQ, pcbuffer, portMAX_DELAY);
        }
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
            memset(pcBuffer, 0, PRINT_UTILS_MAX_BUF);
        }
    }
}

#endif

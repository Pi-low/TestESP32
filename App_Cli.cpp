/**
 * @brief Command Line Interface engine
 * @file App_Cli.cpp
 * @version 0.1
 * @date 2025-11-04
 * @author Nello
 */

#include "App_Cli.h"
#include "App_Leds.h"

#if APP_TASKS
// APP_CLI
#define CLI_TASK            "APP_CLI"
#define CLI_TASK_HEAP       (configMINIMAL_STACK_SIZE*4)
#define CLI_TASK_PARAM      NULL
#define CLI_TASK_PRIO       2
#define CLI_TASK_HANDLE     NULL
#define CLI_PERIOD          100 //ms
void vAppCli_Task(void* pvArg);
#endif

#define CLI_RX_BUFFER_SIZE  128

SimpleCLI xCli;
/* Runtime command */
Command Cli_on;
Command Cli_off;
Command Cli_setBrightness;
Command Cli_setWhiteBalance;
Command Cli_setAnim;
Command Cli_setPrescaler;
Command Cli_setPeriod;
Command Cli_setFade;
Command Cli_setDirection;
Command Cli_setOffset;
Command Cli_setBpm;
Command Cli_clearPalette;
Command Cli_addColor;
//Advanced command
// Command Cli_listPalettes;
// Command Cli_selectPalette;
// Command Cli_newPalette;
// Command Cli_createCue;
// Command Cli_playCue;

void vAppCli_init(void) {
    xTaskCreate(vAppCli_Task, CLI_TASK, CLI_TASK_HEAP, CLI_TASK_PARAM, CLI_TASK_PRIO, CLI_TASK_HANDLE);
}

#if APP_TASKS
static void vAppCli_Task(void* pvArg) {
    BaseType_t xLastWakeTime = xTaskGetTickCount();
    char tcRxBuffer[CLI_RX_BUFFER_SIZE] = {0};
    while (1) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CLI_PERIOD));
        if (Serial.available()) {
            Serial.readBytesUntil('\n', tcRxBuffer, CLI_RX_BUFFER_SIZE-1);
            xCli.parse(tcRxBuffer);
        }
    }
}
#endif




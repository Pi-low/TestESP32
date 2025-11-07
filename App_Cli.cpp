/**
 * @brief Command Line Interface engine
 * @file App_Cli.cpp
 * @version 0.1
 * @date 2025-11-04
 * @author Nello
 */

#include "App_Cli.h"
#include "App_Leds.h"
#include "App_PrintUtils.h"
#include <string>

#if APP_TASKS
// APP_CLI
#define CLI_TASK            "APP_CLI"
#define CLI_TASK_HEAP       (configMINIMAL_STACK_SIZE*4)
#define CLI_TASK_PARAM      NULL
#define CLI_TASK_PRIO       2
#define CLI_TASK_HANDLE     NULL
#define CLI_PERIOD          100 //ms
static void vAppCli_Task(void* pvArg);
#endif

#define GENERATE_CMD_ENUM(ENUM) Cmd_##ENUM,
#define GENERATE_STRING(STRING) #STRING,

#define GET_CMD_ENUM(CMD) Cmd_##CMD
#define GET_CMD_WORD(CMD) tpcCommands[Cmd_##CMD]
#define GET_CMD_OBJ(CMD) xCommands[Cmd_##CMD]
#define NB_COMMANDS 13

#define CLI_RX_BUFFER_SIZE  128
#define CLI_TX_BUFFER_SIZE  128

#define FOREACH_CLI_CMD(PARAM)  \
    PARAM(on)                   \
    PARAM(off)                  \
    PARAM(brightness)           \
    PARAM(whiteCorrect)         \
    PARAM(anim)                 \
    PARAM(speed)                \
    PARAM(fade)                 \
    PARAM(direction)            \
    PARAM(offset)               \
    PARAM(bpm)                  \
    PARAM(palette)              \
    PARAM(addColor)             \
    PARAM(color)



typedef enum {
    FOREACH_CLI_CMD(GENERATE_CMD_ENUM)
} TeCliCmd;

static const char *tpcCommands[NB_COMMANDS] = {
    FOREACH_CLI_CMD(GENERATE_STRING)
};

SimpleCLI xCli;

static Command xCommands[NB_COMMANDS];
static char tcCLI_WriteBuffer[CLI_TX_BUFFER_SIZE];

static void vCallback_off(cmd* xCommand);
static void vCallback_on(cmd* xCommand);
static void vCallback_brightness(cmd* xCommand);

void vAppCli_init(void) {
    GET_CMD_OBJ(on) = xCli.addBoundlessCommand("on", vCallback_on);
    GET_CMD_OBJ(off) = xCli.addBoundlessCommand("off", vCallback_off);
    GET_CMD_OBJ(brightness) = xCli.addBoundlessCommand("brightness", vCallback_brightness);
    xTaskCreate(vAppCli_Task, CLI_TASK, CLI_TASK_HEAP, CLI_TASK_PARAM, CLI_TASK_PRIO, CLI_TASK_HANDLE);
}

#if APP_TASKS
static void vAppCli_Task(void* pvArg) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
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

static void vCallback_off(cmd* xCommand) {
    Command cmd(xCommand);
    String cmdName = cmd.getName();
    if (bAppLed_blackout()) {
        snprintf(tcCLI_WriteBuffer, CLI_TX_BUFFER_SIZE, "[Cli] %s: Ok\r\n", cmdName.c_str());
        APP_TRACE(tcCLI_WriteBuffer);
    }
    else {
        snprintf(tcCLI_WriteBuffer, CLI_TX_BUFFER_SIZE, "[Cli] %s: Error !\r\n", cmdName.c_str());
        APP_TRACE(tcCLI_WriteBuffer);
    }
}

static void vCallback_on(cmd* xCommand) {
    Command cmd(xCommand);
    String cmdName = cmd.getName();
    if (bAppLed_resume()) {
        snprintf(tcCLI_WriteBuffer, CLI_TX_BUFFER_SIZE, "[Cli] %s: Ok\r\n", cmdName.c_str());
        APP_TRACE(tcCLI_WriteBuffer);
    }
    else {
        snprintf(tcCLI_WriteBuffer, CLI_TX_BUFFER_SIZE, "[Cli] %s: Error !\r\n", cmdName.c_str());
        APP_TRACE(tcCLI_WriteBuffer);
    }
}

static void vCallback_brightness(cmd* xCommand) {
    Command cmd(xCommand);
    Argument Arg = cmd.getArgument(0);
    String strArg = Arg.getValue();
    uint8_t u8Brightness = strArg.toInt();
    if (bAppLed_setBrightness(u8Brightness)) {
        snprintf(tcCLI_WriteBuffer, CLI_TX_BUFFER_SIZE, "[Cli] %s: %u\r\n", cmd.getName().c_str(), u8Brightness);
        APP_TRACE(tcCLI_WriteBuffer);
    }
    else {
        snprintf(tcCLI_WriteBuffer, CLI_TX_BUFFER_SIZE, "[Cli] %s: Error !\r\n", cmd.getName().c_str());
        APP_TRACE(tcCLI_WriteBuffer);
    }
}

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

#define CLI_RX_BUFFER_SIZE  128
#define CLI_TX_BUFFER_SIZE  128

#define GENERATE_CMD_ENUM(ENUM)         Cmd_##ENUM,
#define CMD_OBJ(CMD)                    xCommands[Cmd_##CMD]
#define CMD_CALLBACK(CMD)               xCommands[Cmd_##CMD] = xCli.addBoundlessCommand(#CMD, vCallback_##CMD)

#define FOREACH_CLI_CMD(PARAM)          \
    PARAM(on)                           \
    PARAM(off)                          \
    PARAM(brightness)                   \
    PARAM(whiteCorrect)                 \
    PARAM(anim)                         \
    PARAM(speed)                        \
    PARAM(period)                       \
    PARAM(fade)                         \
    PARAM(direction)                    \
    PARAM(offset)                       \
    PARAM(bpm)                          \
    PARAM(palette)                      \
    PARAM(addColor)                     \
    PARAM(color)

#define NB_COMMANDS 14

typedef enum {
    FOREACH_CLI_CMD(GENERATE_CMD_ENUM)
} TeCliCmd;

SimpleCLI xCli;
static void vCallback_off(cmd* xCommand);
static void vCallback_on(cmd* xCommand);
static void vCallback_brightness(cmd* xCommand);
static void vCallback_anim(cmd* xCommand);
static void vCallback_speed(cmd* xCommand);
static void vCallback_period(cmd* xCommand);
static void vCallback_fade(cmd* xCommand);

static void vAppCli_SendResponse(const char* pcCommandName, bool bResult, const char* pcExtraString);
static void vCallback_error(cmd* xCommand);

static Command xCommands[NB_COMMANDS];
static char tcCLI_WriteBuffer[CLI_TX_BUFFER_SIZE];

static void vCallback_off(cmd* xCommand);
static void vCallback_on(cmd* xCommand);
static void vCallback_brightness(cmd* xCommand);

void vAppCli_init(void) {
    CMD_CALLBACK(on);
    CMD_CALLBACK(off);
    CMD_CALLBACK(brightness);
    CMD_CALLBACK(speed);
    CMD_CALLBACK(period);
    CMD_CALLBACK(fade);
    xTaskCreate(vAppCli_Task, CLI_TASK, CLI_TASK_HEAP, CLI_TASK_PARAM, CLI_TASK_PRIO, CLI_TASK_HANDLE);
    APP_TRACE(">");
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
    vAppCli_SendResponse(cmdName.c_str(), bAppLed_blackout(), NULL);
}

void vCallback_on(cmd* xCommand) {
    Command cmd(xCommand);
    String cmdName = cmd.getName();
    vAppCli_SendResponse(cmdName.c_str(), bAppLed_resume(), NULL);
}

void vCallback_brightness(cmd* xCommand) {
    Command cmd(xCommand);
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint8_t u8Value = argStr.toInt();
    vAppCli_SendResponse(cmd.getName().c_str(), bAppLed_SetBrightness(u8Value), argStr.c_str());
}

void vCallback_anim(cmd* xCommand) {
    Command cmd(xCommand);
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    vAppCli_SendResponse(cmd.getName().c_str(), bAppLed_SetBrightness(u8Value), argStr.c_str());
}

void vCallback_speed(cmd* xCommand) {
    Command cmd(xCommand);
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint8_t u8Value = argStr.toInt();
    vAppCli_SendResponse(cmd.getName().c_str(), bAppLed_SetSpeed(u8Value), argStr.c_str());
}

void vCallback_period(cmd* xCommand) {
    Command cmd(xCommand);
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint32_t u32Value = argStr.toInt();
    vAppCli_SendResponse(cmd.getName().c_str(), bAppLed_SetPeriod(u32Value), argStr.c_str());
}

void vCallback_fade(cmd* xCommand) {
    Command cmd(xCommand);
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint8_t u8Value = argStr.toInt();
    vAppCli_SendResponse(cmd.getName().c_str(), bAppLed_SetFade(u8Value), argStr.c_str());
}

void vAppCli_SendResponse(const char* pcCommandName, bool bResult, const char* pcExtraString) {
    snprintf(tcCLI_WriteBuffer, CLI_TX_BUFFER_SIZE, "%s: %s %s\r\n>", pcCommandName, bResult ? "ok" : "errror", pcExtraString ? pcExtraString : "\0");
    APP_TRACE(tcCLI_WriteBuffer);
}


void vCallback_error(cmd* xCommand) {

}

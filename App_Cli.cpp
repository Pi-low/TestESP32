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
#define SET_BOUNDLESS(CMD)              xCommands[Cmd_##CMD] = xCli.addBoundlessCmd(#CMD, vCallback_##CMD)
#define SET_SINGLE(CMD)                 xCommands[Cmd_##CMD] = xCli.addSingleArgCmd(#CMD, vCallback_##CMD)
#define SET_MULTI(CMD)                  xCommands[Cmd_##CMD] = xCli.addCmd(#CMD, vCallback_##CMD)

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
    PARAM(config)                       \
    PARAM(set)                          \
    PARAM(setWifi)                      \
    PARAM(setMqtt)                      


#define NB_COMMANDS 17

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
static void vCallback_direction(cmd* xCommand);
static void vCallback_offset(cmd* xCommand);
static void vCallback_bpm(cmd* xCommand);
static void vCallback_palette(cmd* xCommand);
static void vCallback_addColor(cmd* xCommand);
static void vCallback_config(cmd* xCommand);
static void vCallback_set(cmd* xCommand);
static void vCallback_setWifi(cmd* xCommand);
static void vCallback_setMqtt(cmd* xCommand);

static void vAppCli_SendResponse(const char* pcCommandName, eApp_RetVal eRetval, const char* pcExtraString);
static char* pcReturnValueToString(eApp_RetVal eRet);
static void vCallback_error(cmd_error* xError);

static Command xCommands[NB_COMMANDS];
static char tcCLI_WriteBuffer[CLI_TX_BUFFER_SIZE];

const char* CtcAppCli_argMqtt[] = {
    "id",
    "addr",
    "port",
    "login",
    "pwd",
    "topic",
    "keepAlive"
};

const char *CtcAppCli_argSet[] = {
    "deviceName",
    "strips"
};

void vAppCli_init(void) {
    SET_BOUNDLESS(on);
    SET_BOUNDLESS(off);
    SET_BOUNDLESS(brightness);
    SET_BOUNDLESS(anim);
    SET_BOUNDLESS(speed);
    SET_BOUNDLESS(period);
    SET_BOUNDLESS(fade);
    SET_BOUNDLESS(direction);
    SET_BOUNDLESS(offset);
    SET_BOUNDLESS(bpm);
    SET_BOUNDLESS(palette);
    SET_BOUNDLESS(addColor);
    SET_SINGLE(config);
    SET_MULTI(set);
    SET_MULTI(setWifi);
    SET_MULTI(setMqtt);

    CMD_OBJ(set).addArg("deviceName", nullptr);
    CMD_OBJ(set).addArg("strips", nullptr);
    

    CMD_OBJ(setWifi).addArg("ssid", nullptr);
    CMD_OBJ(setWifi).addArg("pwd", nullptr);

    CMD_OBJ(setMqtt).addArg("id", nullptr);
    CMD_OBJ(setMqtt).addArg("addr", nullptr);
    CMD_OBJ(setMqtt).addArg("port", nullptr);
    CMD_OBJ(setMqtt).addArg("login", nullptr);
    CMD_OBJ(setMqtt).addArg("pwd", nullptr);
    CMD_OBJ(setMqtt).addArg("topic", nullptr);
    CMD_OBJ(setMqtt).addArg("keepAlive", nullptr);

    xCli.setErrorCallback(vCallback_error);

    xTaskCreate(vAppCli_Task, CLI_TASK, CLI_TASK_HEAP, CLI_TASK_PARAM, CLI_TASK_PRIO, CLI_TASK_HANDLE);
    APP_TRACE("\r\n>");
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
            while(Serial.available()) { Serial.read(); } //flush rest of buffer
            memset(tcRxBuffer, 0, CLI_RX_BUFFER_SIZE);
        }
    }
}
#endif

static void vCallback_off(cmd* xCommand) {
    Command cmd(xCommand);
    String cmdName = cmd.getName();
    vAppCli_SendResponse(cmdName.c_str(), eAppLed_blackout(), NULL);
}

static void vCallback_on(cmd* xCommand) {
    Command cmd(xCommand);
    String cmdName = cmd.getName();
    vAppCli_SendResponse(cmdName.c_str(), eAppLed_resume(), NULL);
}

static void vCallback_brightness(cmd* xCommand) {
    Command cmd(xCommand);
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint8_t u8Value = argStr.toInt();
    vAppCli_SendResponse(cmd.getName().c_str(), eAppLed_SetBrightness(u8Value), argStr.c_str());
}

static void vCallback_anim(cmd* xCommand) {
    Command cmd(xCommand);
    String StrExtra;
    uint8_t u8Index;

    //animation type
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    SubStrip::TeAnimation eAnim = SubStrip::NONE;
    char **tpcKeyWord = (char**)tpcAppLED_Animations;
    StrExtra += argStr + " ";

    for (uint8_t  i = 0; i < SubStrip::NB_ANIMS; i++) {
        if (argStr.equalsIgnoreCase(*tpcKeyWord)) {
            eAnim = (SubStrip::TeAnimation)i;
            break;
        }
        tpcKeyWord++;
    }

    // index
    xArg = cmd.getArgument(1);
    argStr = xArg.getValue();
    if (argStr.isEmpty())
    { argStr = "all"; }
    StrExtra += argStr;
    u8Index = (argStr.equalsIgnoreCase("all")) ? _LED_ALLSTRIPS : argStr.toInt();

    vAppCli_SendResponse(cmd.getName().c_str(), eAppLed_SetAnimation(eAnim, u8Index), StrExtra.c_str());
}

static void vCallback_speed(cmd* xCommand) {
    Command cmd(xCommand);
    String StrExtra;

    // Set value
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint8_t u8Value = argStr.toInt();
    StrExtra += argStr + " ";

    // Index
    xArg = cmd.getArgument(1);
    argStr = xArg.getValue();
    if (argStr.isEmpty())
    { argStr = "all"; }
    StrExtra += argStr;
    uint8_t u8Index = (argStr.equalsIgnoreCase("all")) ? _LED_ALLSTRIPS : argStr.toInt();

    vAppCli_SendResponse(cmd.getName().c_str(), eAppLed_SetSpeed(u8Value, u8Index), StrExtra.c_str());
}

static void vCallback_period(cmd* xCommand) {
    Command cmd(xCommand);
    String StrExtra;

    // Set value
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint32_t u32Value = argStr.toInt();
    StrExtra += argStr + " ";

    // Index
    xArg = cmd.getArgument(1);
    argStr = xArg.getValue();
    if (argStr.isEmpty())
    { argStr = "all"; }
    StrExtra += argStr;
    uint8_t u8Index = (argStr.equalsIgnoreCase("all")) ? _LED_ALLSTRIPS : argStr.toInt();

    vAppCli_SendResponse(cmd.getName().c_str(), eAppLed_SetPeriod(u32Value, u8Index), StrExtra.c_str());
}

static void vCallback_fade(cmd* xCommand) {
    Command cmd(xCommand);
    String StrExtra;

    // Set value
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint16_t u16Value = argStr.toInt();
    StrExtra += argStr + " ";

    // Index
    xArg = cmd.getArgument(1);
    argStr = xArg.getValue();
    if (argStr.isEmpty())
    { argStr = "all"; }
    StrExtra += argStr;
    uint8_t u8Index = (argStr.equalsIgnoreCase("all")) ? _LED_ALLSTRIPS : argStr.toInt();

    vAppCli_SendResponse(cmd.getName().c_str(), eAppLed_SetFade(u16Value, u8Index), StrExtra.c_str());
}

static void vCallback_direction(cmd* xCommand) {
    Command cmd(xCommand);
    String StrExtra;

    // Set value
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    StrExtra += argStr + " ";
    SubStrip::TeDirection eDirection = SubStrip::FORWARD_INOUT;
    if (argStr.equalsIgnoreCase("fwd")) {
        eDirection = SubStrip::FORWARD_INOUT;
    } else if (argStr.equalsIgnoreCase("rvs")) {
        eDirection = SubStrip::REVERSE_OUTIN;
    }

    // Index
    xArg = cmd.getArgument(1);
    argStr = xArg.getValue();
    if (argStr.isEmpty())
    { argStr = "all"; }
    StrExtra += argStr;
    uint8_t u8Index = (argStr.equalsIgnoreCase("all")) ? _LED_ALLSTRIPS : argStr.toInt();

    vAppCli_SendResponse(cmd.getName().c_str(), eAppLed_SetDirection(eDirection, u8Index), StrExtra.c_str());
}

static void vCallback_offset(cmd* xCommand) {
    Command cmd(xCommand);
    String StrExtra;

    // Set value
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint8_t u8Value = argStr.toInt();
    StrExtra += argStr + " ";

    // Index
    xArg = cmd.getArgument(1);
    argStr = xArg.getValue();
    if (argStr.isEmpty())
    { argStr = "all"; }
    StrExtra += argStr;
    uint8_t u8Index = (argStr.equalsIgnoreCase("all")) ? _LED_ALLSTRIPS : argStr.toInt();

    vAppCli_SendResponse(cmd.getName().c_str(), eAppLed_SetOffset(u8Value, u8Index), StrExtra.c_str());
}

static void vCallback_bpm(cmd* xCommand) {
    Command cmd(xCommand);
    String StrExtra;

    // Set value
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint8_t u8Value = argStr.toInt();
    StrExtra += argStr + " ";

    // Index
    xArg = cmd.getArgument(1);
    argStr = xArg.getValue();
    if (argStr.isEmpty())
    { argStr = "all"; }
    StrExtra += argStr;
    uint8_t u8Index = (argStr.equalsIgnoreCase("all")) ? _LED_ALLSTRIPS : argStr.toInt();

    vAppCli_SendResponse(cmd.getName().c_str(), eAppLed_SetBpm(u8Value, u8Index), StrExtra.c_str());
}

static void vCallback_palette(cmd* xCommand) {
    Command cmd(xCommand);
    String StrExtra = "Substrip[";

    // Strip index
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    StrExtra += argStr + "] <= Palette[";
    uint8_t u8StripIndex = (argStr.equalsIgnoreCase("all")) ? _LED_ALLSTRIPS : argStr.toInt();

    // Palette index
    xArg = cmd.getArgument(1);
    argStr = xArg.getValue();
    if (argStr.isEmpty())
    { argStr = "0"; }
    StrExtra += argStr + "]";
    uint8_t u8PaletteIndex = argStr.toInt();

    vAppCli_SendResponse(cmd.getName().c_str(), eAppLed_SetPalette(u8PaletteIndex, u8StripIndex), StrExtra.c_str());
}

static void vCallback_addColor(cmd* xCommand) {
    uint8_t u8ColorCnt = 0;
    uint8_t i;
    CRGB tcTmpPalette[6];
    Command cmd(xCommand);
    String StrExtra = "Palette[";


    // Palette index
    Argument xArg = cmd.getArgument(0);
    String argStr = xArg.getValue();
    uint8_t u8PaletteIndex = argStr.toInt();
    StrExtra += argStr + "] = ";

    do {
        xArg = cmd.getArgument(u8ColorCnt + 1);
        argStr = xArg.getValue();
        uint32_t u32Color = (uint32_t)strtol(argStr.c_str(), NULL, 16);
        tcTmpPalette[u8ColorCnt] = CRGB((uint8_t)(u32Color >> 16), (uint8_t)(u32Color >> 8), (uint8_t)(u32Color));
        StrExtra += String(u32Color, HEX) + " ";
        u8ColorCnt++;
    } while ((u8ColorCnt < 6) && !argStr.isEmpty());

    vAppCli_SendResponse(cmd.getName().c_str(), eAppLed_LoadColors(tcTmpPalette, u8ColorCnt, u8PaletteIndex), StrExtra.c_str());
}

static void vAppCli_SendResponse(const char* pcCommandName, eApp_RetVal eRetval, const char* pcExtraString) {
    snprintf(tcCLI_WriteBuffer, CLI_TX_BUFFER_SIZE, "%s: %s %s\r\n>", pcReturnValueToString(eRetval), pcCommandName, pcExtraString ? pcExtraString : "\0");
    APP_TRACE(tcCLI_WriteBuffer);
}

static char* pcReturnValueToString(eApp_RetVal eRet) {
    switch (eRet) {
        case eRet_Warning:
            return (char*)"Warning";
        case eRet_Ok:
            return (char*)"Ok";
        case eRet_Error:
            return (char*)"Error";
        case eRet_BadParameter:
            return (char*)"Bad parameter";
        case eRet_InternalError:
            return (char*)"Internal error";
        case eRet_JsonError:
            return (char*)"Json error";
        default:
            return (char*)"Unknown";
    }
}

static void vCallback_config(cmd* xCommand)
{
    Command cmd(xCommand);
    Argument arg = cmd.getArgument(0);
    eApp_RetVal eRet = eRet_Ok;

    if (arg.getValue().operator==("reset"))
    {
        APP_TRACE("Resetting config file...\r\n");
        remove(CONFIG_FILE_PATH);
        if (eAppCfg_SetDefaultConfig() < eRet_Ok)
        {
            APP_TRACE("Could not set default config !");
        }
        else
        {
            eAppCfg_SaveConfig(CONFIG_FILE_PATH);
        }
    }
    else if (arg.getValue().operator==("print"))
    {
        uint16_t u16PrettyCfg_Size = measureJsonPretty(jAppCfg_Config);
        char *pcPrettyConfig = (char *)pvPortMalloc(u16PrettyCfg_Size);
        APP_TRACE("Print pretty config:\r\n");
        if (pcPrettyConfig)
        {
            serializeJsonPretty(jAppCfg_Config, pcPrettyConfig, u16PrettyCfg_Size);
            vAppPrintUtils_Print(pcPrettyConfig, u16PrettyCfg_Size);
            vPortFree(pcPrettyConfig);
        }
        else
        {
            APP_TRACE("Malloc error !!");
        }
    }
    else if (arg.getValue().operator==("save"))
    {
        eAppCfg_SaveConfig(CONFIG_FILE_PATH);
    }
    else
    {
        APP_TRACE("Unknown argument!!");
    }
    APP_TRACE("\r\n>");
}

static void vCallback_set(cmd* xCommand)
{
    Command cmd(xCommand);
    Argument arg;
    char **pcArgList = (char**)CtcAppCli_argSet;
    char pcTmp[128] = {0};

    for (uint8_t i = 0; i < (sizeof(CtcAppCli_argSet) / sizeof(CtcAppCli_argSet[0])); i++)
    {
        arg = cmd.getArg(*pcArgList);
        if (arg.isSet())
        {
            snprintf(pcTmp, 128, "%s = %s\r\n", *pcArgList, arg.getValue().c_str());
            APP_TRACE(pcTmp);
            switch (i)
            {
            case 0: // deviceName
                bAppCfg_LockJson();
                jAppCfg_Config["DEVICE_NAME"] = arg.getValue();
                bAppCfg_UnlockJson();
                break;

            case 1: //strips
                eAppCfg_SetStrips(arg.getValue().c_str());
            break;
            }
        }
        pcArgList++;
    }
}

static void vCallback_setWifi(cmd* xCommand)
{
    Command cmd(xCommand);
    Argument arg_ssid = cmd.getArg("ssid");
    Argument arg_pwd = cmd.getArg("pwd");
    char pcTmp[64] = {0};
    bAppCfg_LockJson();
    if (arg_ssid.isSet())
    {
        jAppCfg_Config["WIFI"]["SSID"] = arg_ssid.getValue();
        snprintf(pcTmp, 64, "Set wifi.ssid: %s\r\n", arg_ssid.getValue().c_str());
        APP_TRACE(pcTmp);
    }
    if (arg_pwd.isSet())
    {
        jAppCfg_Config["WIFI"]["PWD"] = arg_pwd.getValue();
        snprintf(pcTmp, 64, "Set wifi.pwd: %s\r\n", arg_pwd.getValue().c_str());
        APP_TRACE(pcTmp);
    }
    bAppCfg_UnlockJson();
    APP_TRACE("\r\n>");
}

static void vCallback_setMqtt(cmd* xCommand)
{
    Command cmd(xCommand);
    uint8_t u8Index = 0;
    char pcTmp[64] = {0};
    Argument arg;
    char** pcArgLst;
    String str_args[7];
    do 
    {
        arg = cmd.getArg(u8Index);
        if (arg.isSet())
        {
            pcArgLst = (char **)CtcAppCli_argMqtt;
            for (uint8_t i = 0; i < sizeof(CtcAppCli_argMqtt) / sizeof(CtcAppCli_argMqtt[0]); i++)
            {
                if (arg.getName().operator==(*pcArgLst))
                {
                    snprintf(pcTmp, 64, "Set mqtt.%s: %s\r\n", arg.getName().c_str(), arg.getValue().c_str());
                    APP_TRACE(pcTmp);
                    str_args[i] = arg.getValue();
                    break;
                }
                pcArgLst++;
            }
        }
        u8Index++;
    } while (u8Index < cmd.countArgs());
    bAppCfg_LockJson();
    if (!str_args[0].isEmpty())
    { jAppCfg_Config["MQTT"]["ID"] = str_args[0]; }
    if (!str_args[1].isEmpty())
    { jAppCfg_Config["MQTT"]["ADDR"] = str_args[1]; }
    if (!str_args[2].isEmpty())
    { jAppCfg_Config["MQTT"]["PORT"] = atoi(str_args[2].c_str()); }
    if (!str_args[3].isEmpty())
    { jAppCfg_Config["MQTT"]["LOGIN"] = str_args[3]; }
    if (!str_args[4].isEmpty())
    { jAppCfg_Config["MQTT"]["PWD"] = str_args[4]; }
    if (!str_args[5].isEmpty())
    { jAppCfg_Config["MQTT"]["GLOBAL_TOPIC"] = str_args[5]; }
    if (!str_args[6].isEmpty())
    { jAppCfg_Config["MQTT"]["KEEPALIVE"] = atoi(str_args[6].c_str()); }
    bAppCfg_UnlockJson();
    APP_TRACE("\r\n>");
}

static void vCallback_error(cmd_error* xError) {
    CommandError cmdError(xError);
    String ErrStr = cmdError.toString();
    vAppCli_SendResponse(ErrStr.c_str(), eRet_Error, NULL);
}

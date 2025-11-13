

/**
 * @file Config.cpp
 * @brief Application configuration implementation
 * @version 0.1
 * @date 2025-11-11
 * @author Nello
 */

#include "App_PrintUtils.h"
#include "Config.h"
#include "FS.h"
#include "FFat.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#define CONFIG_FILE_PATH    "/config.cfg"

typedef enum {
    TYPE_JSON_NUMBER,
    TYPE_JSON_STRING,
    TYPE_JSON_ARRAY,
    TYPE_JSON_OBJECT
} TeJsonType;

#define _MNG_RETURN(x)  eRet = x
#define GEN_STRING(VALUE)                   #VALUE,
#define GEN_ENUM(VALUE)                     Cfg_##VALUE,

#define FOREACH_CONFIG_OBJ(PARAM)           \
    PARAM(DEVICE_NAME)                      \
    PARAM(WIFI)                             \
    PARAM(MQTT)                             \
    PARAM(TOKEN)                            \
    PARAM(DEVICE_SUBSTRIPS)                 \
    PARAM(DEVICE_PALETTES)                  \
    PARAM(DEVICE_PROG_ANIM)                 \
    PARAM(DEVICE_WORKING_TIMESLOT)          \
    PARAM(DEVICE_LAST_CONTEXT)

#define CFG_NB_OBJ                          9

static String strCfgObjects[] = {FOREACH_CONFIG_OBJ(GEN_STRING)};

typedef enum {
    FOREACH_CONFIG_OBJ(GEN_ENUM)
} TeCfgObject;

const char *tpcAppCfg_Default[CFG_NB_OBJ] = {
    "DEVICE_00",
    R"([{"SSID":null,"PWD": null}])",
    R"({"SERVER":null,"PORT":8883,"CFG_TOPIC":"/lumiapp/config","CMD_TOPIC":"/lumiapp/cmd","KEEPALIVE":60})",
    NULL,
    "[20,20,20,20,20]",
    R"([{"NAME":"default","COLORS":["ffffff","ff0000"]}])",
    R"([{"ANIM":"RAINDROPS","PALETTE":"default","DURATION":60}])",
    R"(["18:00:00","22:00:00"])",
    NULL
};

const TeJsonType cteAppCfg_ObjTypes[CFG_NB_OBJ] = {
    TYPE_JSON_STRING,
    TYPE_JSON_ARRAY,
    TYPE_JSON_OBJECT,
    TYPE_JSON_STRING,
    TYPE_JSON_ARRAY,
    TYPE_JSON_ARRAY,
    TYPE_JSON_ARRAY,
    TYPE_JSON_ARRAY,
    TYPE_JSON_OBJECT
};



JsonDocument<2048> jAppCfg_Config;

eApp_RetVal eAppConfig_init(void) {
    bool bFret;
    bool bDefaultConfig = false;
    char tcWrBuffer[128];
    uint8_t i = 0;
    eApp_RetVal eRet = eRet_Ok;

    while (((bFret = FFat.begin()) == false) && (i < 1)) {
        FFat.format();
        snprintf(tcWrBuffer, 128, "Failed to mount FFat, formatting (%d)...\r\n", i);
        APP_TRACE(tcWrBuffer);
        i++;
    }

    snprintf(tcWrBuffer, 128, "Mount FFat: %s\r\n", bFret ? "success" : "fail");
    APP_TRACE(tcWrBuffer);

    if (bFret)
    {
    }
    else
    { _MNG_RETURN(eRet_InternalError); }
        
    return eRet;
}

eApp_RetVal eAppCfg_LoadConfig(const char* pcFromFilePath) {
    eApp_RetVal eRet = eRet_Ok;
    DeserializationError eError;
    char tcWrBuffer[128];
    File xConfigFile = FFat.open(pcFromFilePath, FILE_READ);
    if (!xConfigFile) {
        APP_TRACE("Config file does not exist...\r\n");
        _MNG_RETURN(eRet_InternalError);
    }
    else if ((eError = deserializeJson(jAppCfg_Config, xConfigFile)) != DeserializationError::Ok) {
        char tcWrBuffer[128];
        _MNG_RETURN(eRet_InternalError);
    }
    snprintf(tcWrBuffer, 128, "Loading config: %s\r\n", eError.c_str());
    APP_TRACE(tcWrBuffer);

    return eRet;
}

eApp_RetVal eAppCfg_SetDefaultConfig(const char* pcToFilePath) {
    eApp_RetVal eRet = eRet_Ok;
    JsonVariant jVar;
    JsonDocument jDoc;
    for (uint8_t i = 0; i < CFG_NB_OBJ; i++) {
        deserializeJson(jDoc, tpcAppCfg_Default[i]);
        jVar = jDoc.as<JsonVariant>();

        switch (cteAppCfg_ObjTypes[i]) {
            case TYPE_JSON_NUMBER:
            jAppCfg_Config[strCfgObjects[i]] = jVar.as<JsonInteger>();
            break;

            case TYPE_JSON_STRING:
            jAppCfg_Config[strCfgObjects[i]] = jVar.as<JsonString>();
            break;

            case TYPE_JSON_ARRAY:
            jAppCfg_Config[strCfgObjects[i]] = jVar.as<JsonArray>();
            break;

            case TYPE_JSON_OBJECT:
            jAppCfg_Config[strCfgObjects[i]] = jVar.as<JsonObject>();
            break;
        };
    }
}



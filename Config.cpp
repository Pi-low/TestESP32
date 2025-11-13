

/**
 * @file Config.cpp
 * @brief Application configuration implementation
 * @version 0.1
 * @date 2025-11-11
 * @author Nello
 */

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "App_PrintUtils.h"
#include "Config.h"
#include "FS.h"
#include "FFat.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

/*******************************************************************************
 *  Types, nums, macros
 ******************************************************************************/
#define CONFIG_FILE_PATH    "/config.cfg"
#define _MNG_RETURN(x)                      eRet = x
#define CFG_NB_OBJ                          9

typedef enum {
    TYPE_JSON_NULL,
    TYPE_JSON_NUMBER,
    TYPE_JSON_STRING,
    TYPE_JSON_ARRAY,
    TYPE_JSON_OBJECT
} TeJsonType;

typedef struct {
    const char* pcName;
    TeJsonType eDataType;
    uint8_t u8ArraySize;
    TeJsonType eArrayType;
    const void* pvDefaultValue;
    int32_t i32DefaultNumber;
} TstAppCfg_ParamObj;

/*******************************************************************************
 *  Global variable
 ******************************************************************************/
const char CtcAppCfg_DefDeviceName[] = "DEVICE_00";
const char CtcAppCfg_DefWifi[] = R"([{"SSID":null,"PWD": null}])";
const char CtcAppCfg_DefMqtt[] = R"({"SERVER":null,"PORT":8883,"CFG_TOPIC":"/lumiapp/config","CMD_TOPIC":"/lumiapp/cmd","KEEPALIVE":60})";
const char CtcAppCfg_DefPalettes[] = R"([{"NAME":"default","COLORS":["ffffff","ff0000"]}])";
const char CtcAppCfg_DefWorkTimeslot = R"(["18:00:00","22:00:00"])";
const int32_t Cti32AppCfg_DefStripAssembly[5] = {20, 20, 20, 20, 20};

const TstAppCfg_ParamObj tstAppCfg_Config[CFG_NB_OBJ] = {
    {
        "DEVICE_NAME",
        TYPE_JSON_STRING,
        0,
        TYPE_JSON_NULL,
        CtcAppCfg_DefDeviceName,
        0
    },
    {
        "WIFI",
        TYPE_JSON_ARRAY,
        1,
        TYPE_JSON_OBJECT,
        CtcAppCfg_DefWifi,
        0
    },
    {
        "MQTT",
        TYPE_JSON_OBJECT,
        0,
        TYPE_JSON_NULL,
        CtcAppCfg_DefMqtt,
        0
    },
    {
        "TOKEN",
        TYPE_JSON_STRING,
        0,
        TYPE_JSON_NULL,
        NULL,
        0
    },
    {
        "DEVICE_SUBSTRIPS",
        TYPE_JSON_ARRAY,
        5,
        TYPE_JSON_NUMBER,
        Cti32AppCfg_DefStripAssembly,
        0
    },
    {
        "DEVICE_PALETTES",
        TYPE_JSON_ARRAY,
        1,
        TYPE_JSON_OBJECT,
        CtcAppCfg_DefPalettes,
        0
    },
    {
        "DEVICE_PROG_ANIM",
        TYPE_JSON_ARRAY,
        0,
        TYPE_JSON_OBJECT,
        NULL,
        0
    },
    {
        "DEVICE_WORKING_TIMESLOT",
        TYPE_JSON_ARRAY,
        2,
        TYPE_JSON_STRING,
        CtcAppCfg_DefWorkTimeslot,
        0
    },
    {
        "DEVICE_LAST_CONTEXT",
        TYPE_JSON_OBJECT,
        0,
        TYPE_JSON_NULL,
        NULL,
        0
    }
};

JsonDocument(2048) jAppCfg_Config;

/*******************************************************************************
 *  Prototypes
 ******************************************************************************/
template < class Y, class T >static void vAppCfg_AddArrayToObject(Y &doc, const char *Childstring, T pValue, uint8_t u8ArraySize);
eApp_RetVal eAppCfg_SetDefaultConfig(const char *pcToFilePath);

/*******************************************************************************
 *  Functions
 ******************************************************************************/

/*******************************************************************************
 * @brief Initialize configuration manager
 * 
 * @return eApp_RetVal 
 ******************************************************************************/
eApp_RetVal eAppConfig_init(void)
{
    bool bFret;
    bool bDefaultConfig = false;
    char tcWrBuffer[128];
    uint8_t i = 0;
    eApp_RetVal eRet = eRet_Ok;

    while (((bFret = FFat.begin()) == false) && (i < 1))
    {
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
    {
        _MNG_RETURN(eRet_InternalError);
    }

    return eRet;
}

/*******************************************************************************
 * @brief Load configuration from file
 * 
 * @param pcFromFilePath 
 * @return eApp_RetVal 
 ******************************************************************************/
eApp_RetVal eAppCfg_LoadConfig(const char *pcFromFilePath)
{
    eApp_RetVal eRet = eRet_Ok;
    DeserializationError eError;
    char tcWrBuffer[128];
    File xConfigFile = FFat.open(pcFromFilePath, FILE_READ);
    if (!xConfigFile)
    {
        APP_TRACE("Config file does not exist...\r\n");
        _MNG_RETURN(eRet_InternalError);
    }
    else if ((eError = deserializeJson(jAppCfg_Config, xConfigFile)) != DeserializationError::Ok)
    {
        char tcWrBuffer[128];
        _MNG_RETURN(eRet_InternalError);
    }
    snprintf(tcWrBuffer, 128, "Loading config: %s\r\n", eError.c_str());
    APP_TRACE(tcWrBuffer);

    return eRet;
}

eApp_RetVal eAppCfg_SaveConfig(const char *pcToFilePath)
{
    eApp_RetVal eRet = eRet_Ok;
    char tcWrBuffer[128];

}


/*******************************************************************************
 * @brief Set default configuration
 * 
 * @param pcToFilePath 
 * @return eApp_RetVal 
 ******************************************************************************/
eApp_RetVal eAppCfg_SetDefaultConfig(const char *pcToFilePath)
{
    eApp_RetVal eRet = eRet_Ok;
    TstAppCfg_ParamObj *pstParam = tstAppCfg_Config;
    DeserializationError eError;
    char tcPrint[128] = "Json deserialization: ";
    for (uint8_t i = 0; i < CFG_NB_OBJ; i++)
    {
        switch (pstParam->eDataType)
        {
        case TYPE_JSON_NUMBER:
            jAppCfg_Config[pstParam->pcName] = (int32_t)pstParam->i32DefaultNumber;
            break;

        case TYPE_JSON_STRING:
            jAppCfg_Config[pstParam->pcName] = (const char *)pstParam->pvDefaultValue;
            break;

        case TYPE_JSON_ARRAY:
            switch (pstParam->eArrayType)
            {
            case TYPE_JSON_NUMBER:
                vAppCfg_AddArrayToObject(jAppCfg_Config, pstParam->pcName, (const int32_t *)pstParam->pvDefaultValue, pstParam->u8ArraySize);
                break;

            case TYPE_JSON_STRING:
                vAppCfg_AddArrayToObject(jAppCfg_Config, pstParam->pcName, (const char **)pstParam->pvDefaultValue, pstParam->u8ArraySize);
                break;

            case TYPE_JSON_OBJECT:
                JsonDocument jSub(256);
                eError = deserializeJson(jSub, (const char *)pstParam->pvDefaultValue);
                vAppCfg_AddArrayToObject(jAppCfg_Config, pstParam->pcName, jSub.as<JsonArray>(), pstParam->u8ArraySize);
                break;

            default:
                break;
            }
            break;

        case TYPE_JSON_OBJECT:
            JsonDocument jSub(256);
            eError = deserializeJson(jSub, pstParam->pvDefaultValue);
            JsonVariant jVar = jSub.as<JsonVariant>();
            jAppCfg_Config[pstParam->pcName] = jVar;
            break;

        default:
            break;
        }

        if (eError != DeserializationError::Ok)
        {
            _MNG_RETURN(eRet_InternalError);
            break;
        }
    }
    strcat(tcPrint, eError.c_str());
    strcat(tcPrint, "\r\n");
    APP_TRACE(tcPrint);
    return eRet;
}

template < class Y, class T >static void vAppCfg_AddArrayToObject(Y &doc, const char *Childstring, T pValue, uint8_t u8ArraySize)
{
    if ((u8ArraySize >= 0) && (pValue != NULL))
    {
        JsonArray jsonItem = doc.createNestedArray(Childstring);

        if (jsonItem.isNull() == false)
        {
            for (uint8_t i = 0; i < u8ArraySize; i++)
            {
                jsonItem.add(pValue[i]);
            }
        }
    }
}



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

/*******************************************************************************
 *  Types, nums, macros
 ******************************************************************************/
#define _MNG_RETURN(x)                      eRet = x
#define CFG_NB_OBJ                          14

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
const char CtcAppCfg_DefCfgTopic[] = "/lumiapp/config";
const char CtcAppCfg_DefCmdTopic[] = "/lumiapp/cmd";
const char CtcAppCfg_DefPalettes[] = R"({[{"NAME":"default","COLORS":["ffffff","ff0000"]}]})";
const char CtcAppCfg_DefProgArr[] = R"({[{"ANIM":"glitter","DURATION":120},{"ANIM":"raindrops","DURATION":120}]})";
const char CtcAppCfg_DefWorkTimeSlot[] = R"(["18:00:00","22:00:00"])";
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
        "SSID",
        TYPE_JSON_STRING,
        0,
        TYPE_JSON_NULL,
        NULL,
        0
    },
    {
        "PWD",
        TYPE_JSON_STRING,
        0,
        TYPE_JSON_NULL,
        NULL,
        0
    },
    {
        "MQTT_ADDR",
        TYPE_JSON_STRING,
        0,
        TYPE_JSON_NULL,
        NULL,
        0
    },
    {
        "MQTT_PORT",
        TYPE_JSON_NUMBER,
        0,
        TYPE_JSON_NULL,
        NULL,
        8883
    },
    {
        "MQTT_CFG_TOPIC",
        TYPE_JSON_STRING,
        0,
        TYPE_JSON_NULL,
        CtcAppCfg_DefCfgTopic,
        0
    },
    {
        "MQTT_CMD_TOPIC",
        TYPE_JSON_STRING,
        0,
        TYPE_JSON_NULL,
        CtcAppCfg_DefCmdTopic,
        0
    },
    {
        "MQTT_KEEPALIVE",
        TYPE_JSON_NUMBER,
        0,
        TYPE_JSON_NULL,
        NULL,
        60
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
        2,
        TYPE_JSON_OBJECT,
        CtcAppCfg_DefProgArr,
        0
    },
    {
        "DEVICE_WORKING_TIMESLOT",
        TYPE_JSON_ARRAY,
        2,
        TYPE_JSON_STRING,
        CtcAppCfg_DefWorkTimeSlot,
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

JsonDocument jAppCfg_Config;

/*******************************************************************************
 *  Prototypes
 ******************************************************************************/
template <class Y, class T>
static void vAppCfg_AddArrayToObject(Y &doc, const char *Childstring, T pValue, uint8_t u8ArraySize);
// eApp_RetVal eAppCfg_SetDefaultConfig(void);

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
        if (eAppCfg_LoadConfig(CONFIG_FILE_PATH) < eRet_Ok)
        {
            if (eAppCfg_SetDefaultConfig() < eRet_Ok)
                _MNG_RETURN(eRet_InternalError);
            else if (eAppCfg_SaveConfig(CONFIG_FILE_PATH) < eRet_Ok)
                _MNG_RETURN(eRet_InternalError);
        }
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
    File xConfigFile = FFat.open(pcFromFilePath, FILE_READ);
    if (!xConfigFile)
    {
        APP_TRACE("Config file does not exist.\r\n");
        _MNG_RETURN(eRet_InternalError);
    }
    else if ((eError = deserializeJson(jAppCfg_Config, xConfigFile)) != DeserializationError::Ok)
    {
        _MNG_RETURN(eRet_InternalError);
        APP_TRACE("deserializeJson error!\r\n");
    }
    else
    {
        xConfigFile.close();
        APP_TRACE("Config loaded!\r\n");
    }

    return eRet;
}

/*******************************************************************************
 * @brief Save configuration to file
 * 
 * @param pcToFilePath 
 * @return eApp_RetVal 
 ******************************************************************************/
eApp_RetVal eAppCfg_SaveConfig(const char *pcToFilePath)
{
    eApp_RetVal eRet = eRet_Ok;

    if (pcToFilePath == NULL)
    {
        _MNG_RETURN(eRet_InternalError);
    }
    else
    {
        File xConfigFile = FFat.open(pcToFilePath, FILE_WRITE);
        if (!xConfigFile)
        {
            _MNG_RETURN(eRet_InternalError);
            APP_TRACE("Cannot open config file!\r\n");
        }
        else
        {
            if (!serializeJson(jAppCfg_Config, xConfigFile))
            {
                _MNG_RETURN(eRet_InternalError);
                APP_TRACE("Failed to save config!\r\n");
            }
            else
            {
                APP_TRACE("Config saved!\r\n");
            }
            xConfigFile.close();
        }
    }

    return eRet;
}

/*******************************************************************************
 * @brief Set default configuration
 * 
 * @return eApp_RetVal 
 ******************************************************************************/
eApp_RetVal eAppCfg_SetDefaultConfig(void)
{
    eApp_RetVal eRet = eRet_Ok;
    TstAppCfg_ParamObj *pstParam = (TstAppCfg_ParamObj*) tstAppCfg_Config;
    DeserializationError eError;

    jAppCfg_Config.clear();

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
        {
            switch (pstParam->eArrayType)
            {
            case TYPE_JSON_NUMBER:
                vAppCfg_AddArrayToObject(jAppCfg_Config, pstParam->pcName, (const int32_t *)pstParam->pvDefaultValue, pstParam->u8ArraySize);
                break;

            case TYPE_JSON_STRING:
                vAppCfg_AddArrayToObject(jAppCfg_Config, pstParam->pcName, (const char **)pstParam->pvDefaultValue, pstParam->u8ArraySize);
                break;

            case TYPE_JSON_OBJECT:
            {
                JsonDocument jSub;
                eError = deserializeJson(jSub, (const char *)pstParam->pvDefaultValue);
                if ((eError == DeserializationError::Ok) && !jSub.isNull())
                {
                    // vAppCfg_AddArrayToObject(jAppCfg_Config, pstParam->pcName, jVar.as<JsonArray>(), pstParam->u8ArraySize);
                    JsonArray jArr = jSub.as<JsonArray();
                    jAppCfg_Config[pstParam->pcName] = jArr;
                }
            }
            break;

            default:
                break;
            } // switch()
        }
        break;

        case TYPE_JSON_OBJECT:
        {
            JsonDocument jSub;
            JsonObject Obj = jAppCfg_Config[pstParam->pcName].to<JsonObject>();
            eError = deserializeJson(jSub, pstParam->pvDefaultValue);
            if ((eError == DeserializationError::Ok) && !Obj.isNull())
                Obj = jSub.as<JsonObject>();
        }
        break;

        default:
            break;
        } //switch()

        if (eError != DeserializationError::Ok)
        {
            _MNG_RETURN(eRet_InternalError);
            APP_TRACE("Set defaultConfig error !\r\n");
            break;
        }
        else
        {
            APP_TRACE(". ");
        }
        pstParam++;
    } //for(;;)
    return eRet;
}

template <class Y, class T>
static void vAppCfg_AddArrayToObject(Y &doc, const char *Childstring, T pValue, uint8_t u8ArraySize)
{
    if ((u8ArraySize >= 0) && (pValue != NULL))
    {
        auto jsonItem = doc[Childstring].template to<JsonArray>();

        if (!jsonItem.isNull())
        {
            for (uint8_t i = 0; i < u8ArraySize; i++)
            {
                jsonItem.add(pValue[i]);
            }
        }
    }
}

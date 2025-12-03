

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
static SemaphoreHandle_t xJsonMutex;
const char CtcAppCfg_DefDeviceName[] = "DEVICE_00";
const char CtcAppCfg_DefWifi[] = R"({"SSID":null,"PWD":null})";
const char CtcAppCfg_DefMqtt[] = R"({"ADDR":null,"PORT":null,"CFG_TOPIC":"/lumiapp/config","CMD_TOPIC":"/lumiapp/cmd","KEEPALIVE":60})";
const char CtcAppCfg_DefPalettes[] = R"([{"NAME":"default","COLORS":["ffffff","ff0000"]}])";
const char CtcAppCfg_DefProgArr[] = R"([{"ANIM":"glitter","DURATION":120},{"ANIM":"raindrops","DURATION":120}])";
const char CtcAppCfg_DefWorkTimeSlot[] = R"([{"ON":"17:30:00","OFF":"22:00:00"},{"ON":"06:30:00","OFF":"08:00:00"}])";
// const int32_t Cti32AppCfg_DefStripAssembly[5] = {20, 20, 20, 20, 20};

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
        TYPE_JSON_OBJECT,
        0,
        TYPE_JSON_NULL,
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
        0,
        TYPE_JSON_NUMBER,
        NULL,
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
        TYPE_JSON_OBJECT,
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
eApp_RetVal eAppCfg_ResetParam(TstAppCfg_ParamObj *FpstParam);
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
    xJsonMutex = xSemaphoreCreateMutex();

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
 * @brief Lock json mutex
 * 
 * @return true
 ******************************************************************************/
bool bAppCfg_LockJson(void)
{
    bool bRet = false;
    if (xJsonMutex != NULL)
    {
        bRet = xSemaphoreTake(xJsonMutex, portMAX_DELAY);
    }
    return bRet;
}

/*******************************************************************************
 * @brief Unlock json mutex
 * 
 * @return true 
 ******************************************************************************/
bool bAppCfg_UnlockJson(void)
{
    bool bRet = false;
    if (xJsonMutex != NULL)
    {
        bRet = xSemaphoreGive(xJsonMutex);
    }
    return bRet;
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
    else if (bAppCfg_LockJson())
    {
        if (deserializeJson(jAppCfg_Config, xConfigFile) != DeserializationError::Ok)
        {
            APP_TRACE("Deserialization error.\r\n");
            _MNG_RETURN(eRet_JsonError);
        }
        else
        { APP_TRACE("Config loaded!\r\n"); }
        bAppCfg_UnlockJson();
        xConfigFile.close();
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
            if (bAppCfg_LockJson())
            {
                if (!serializeJson(jAppCfg_Config, xConfigFile))
                {
                    _MNG_RETURN(eRet_JsonError);
                    APP_TRACE("Failed to save config!\r\n");
                }
                else
                { APP_TRACE("Config saved!\r\n"); }
                bAppCfg_UnlockJson();
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
    TstAppCfg_ParamObj *pstParam = (TstAppCfg_ParamObj *)tstAppCfg_Config;
    jAppCfg_Config.clear();

    for (uint8_t i = 0; (i < CFG_NB_OBJ) && (eRet >= eRet_Ok); i++)
    {
        eRet = eAppCfg_ResetParam(pstParam);
        if (eRet < eRet_Ok)
        {
            char tcPrint[40];
            snprintf(tcPrint, 40, "Set defaultConfig error: (%d)", eRet);
            APP_TRACE(tcPrint);
        }
        else
        {
            APP_TRACE(". ");
        }
        pstParam++;
    }
    APP_TRACE("\r\n");
    return eRet;
}

/*******************************************************************************
 * @brief Reset specific parameter from config
 * 
 * @param pcObjectKey 
 * @return eApp_RetVal 
 ******************************************************************************/
eApp_RetVal eAppCfg_ResetParamKey(const char* pcObjectKey)
{
    eApp_RetVal eRet = eRet_Ok;
    TstAppCfg_ParamObj *pstParam = (TstAppCfg_ParamObj *)tstAppCfg_Config;
    uint8_t i = 0;

    // Check existing param key from default config
    while ((strcmp(pcObjectKey, pstParam->pcName) != 0) && (i < CFG_NB_OBJ))
    {
        pstParam++;
        i++;
    }

    if (i >= CFG_NB_OBJ)
    {   // Object param not found
        _MNG_RETURN(eRet_InternalError);
    }
    else
    {
        eRet = eAppCfg_ResetParam(pstParam);
    }
    return eRet;
}

eApp_RetVal eAppCfg_ResetParam(TstAppCfg_ParamObj *FpstParam)
{
    eApp_RetVal eRet = eRet_Ok;
    if (FpstParam == NULL)
    { _MNG_RETURN(eRet_InternalError); }
    else if (bAppCfg_LockJson())
    {
        switch (FpstParam->eDataType)
        {
        case TYPE_JSON_NUMBER:
            jAppCfg_Config[FpstParam->pcName] = (int32_t)FpstParam->i32DefaultNumber;
            break;

        case TYPE_JSON_STRING:
            jAppCfg_Config[FpstParam->pcName] = (FpstParam->pvDefaultValue != NULL) ? (const char *)FpstParam->pvDefaultValue : nullptr;
            break;

        case TYPE_JSON_ARRAY:
        {
            if ((FpstParam->pvDefaultValue != NULL) || FpstParam->u8ArraySize)
            {
                switch (FpstParam->eArrayType)
                {
                case TYPE_JSON_NUMBER:
                    vAppCfg_AddArrayToObject(jAppCfg_Config, FpstParam->pcName, (const int32_t *)FpstParam->pvDefaultValue, FpstParam->u8ArraySize);
                    break;

                case TYPE_JSON_STRING:
                    vAppCfg_AddArrayToObject(jAppCfg_Config, FpstParam->pcName, (const char **)FpstParam->pvDefaultValue, FpstParam->u8ArraySize);
                    break;

                case TYPE_JSON_OBJECT:
                {
                    JsonDocument jSub;
                    if (deserializeJson(jSub, (const char *)FpstParam->pvDefaultValue) != DeserializationError::Ok)
                    { _MNG_RETURN(eRet_JsonError); }
                    else
                    { jAppCfg_Config[FpstParam->pcName] = jSub.as<JsonArray>(); }
                }
                break;

                default:
                    jAppCfg_Config[FpstParam->pcName] = nullptr;
                    break;
                } // switch()
            }
            else
            { jAppCfg_Config[FpstParam->pcName] = nullptr; }
        }
        break;

        case TYPE_JSON_OBJECT:
        {
            if (FpstParam->pvDefaultValue != NULL)
            {
                JsonDocument jSub;
                if ((deserializeJson(jSub, (const char*)FpstParam->pvDefaultValue) != DeserializationError::Ok))
                { _MNG_RETURN(eRet_JsonError); }
                else
                { jAppCfg_Config[FpstParam->pcName] = jSub.as<JsonObject>();  }
            }
            else
            {  jAppCfg_Config[FpstParam->pcName] = nullptr; }
        }
        break;

        default:
            jAppCfg_Config[FpstParam->pcName] = nullptr;
            break;
        } // switch()
        bAppCfg_UnlockJson();
    }
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

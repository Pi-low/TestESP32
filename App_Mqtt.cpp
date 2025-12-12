/**
 * @brief MQTT application
 * @file App_MQTT.cpp
 * @version 0.1
 * @date 2025-12-11
 * @author Nello
 */

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "App_MQTT.h"

#if defined(APP_MQTT) && APP_MQTT
#include "App_PrintUtils.h"
#include "App_Leds.h"
/*******************************************************************************
 *  Types, nums, macros
 ******************************************************************************/
#define MQTT_TOPIC_MAX_LENGTH   128

typedef enum {
    eAppMqtt_Pub,
    eAppMqtt_Sub
} TeAppMqtt_TopicType;

typedef struct {
    TeAppMqtt_Topics        eId;
    MessageReceivedCallback pfCallback;
    char                    tcTopic[MQTT_TOPIC_MAX_LENGTH];
} TstAppMqtt_TopicHandle;

typedef struct {
    bool bAvailable;
    TickType_t xTimeout;
    char tcId[CFG_PARAM_MAX_LEN];
    char tcBroker[CFG_PARAM_MAX_LEN];
    char tcLogin[CFG_PARAM_MAX_LEN];
    char tcPwd[CFG_PARAM_MAX_LEN];
    char tcTopic[CFG_PARAM_MAX_LEN];
    uint16_t u16Port;
    uint16_t u16KeepAlive;
} TstAppMqtt_Config;

/*******************************************************************************
 *  Global variable
 ******************************************************************************/
const char* CAcert = nullptr;
static bool bAppMqtt_ConfOnce;
static TstAppMqtt_Config stAppMqtt_Cfg;

static TstAppMqtt_TopicHandle tstTopicHandles[eAppMqtt_Topic_Nb];
/* Setup on mesage callbacks */
static MessageReceivedCallback tpfAppMqtt_Callbacks[eAppMqtt_Topic_Nb] = {
    pvAppLed_CallbackEvent,     //eAppMqtt_Topic_Event
    pvAppLed_CallbackCmd        //eAppMqtt_Topic_Cmd,
};

/*******************************************************************************
 *  Prototypes
 ******************************************************************************/
static eApp_RetVal eAppMqtt_initTopics(void);
static void vAppMqtt_Subscribe(TstAppMqtt_TopicHandle *pstMqttHandle, size_t xSize);

/*******************************************************************************
 *  Functions
 ******************************************************************************/
void vAppMqtt_init(void)
{
    memset(tstTopicHandles, 0, sizeof(tstTopicHandles));
    bAppMqtt_ConfOnce = false;
    if (bAppMqtt_SyncConfig())
    {
        eAppMqtt_initTopics();
    }
}

void vAppMqtt_connect(void)
{
    if (!stAppMqtt_Cfg.bAvailable)
    {
        if (bAppMqtt_SyncConfig())
        {
            eAppMqtt_initTopics();
        }
    }
    if (!bAppMqtt_ConfOnce && stAppMqtt_Cfg.bAvailable)
    {
        char tcURI[128];
        char tcPrint[256];
        if (stAppMqtt_Cfg.u16KeepAlive)
        {
            mqttClient.setKeepAlive(stAppMqtt_Cfg.u16KeepAlive);
        }
        if (stAppWifi_Config.tcId[0] != '\0')
        {
            mqttClient.setMqttClientName(stAppWifi_Config.tcId);
        }
        snprintf(tcURI, sizeof(tcURI), "mqtts://%s:%u", stAppMqtt_Cfg.tcBroker, stAppMqtt_Cfg.u16Port);
        snprintf(tcPrint, sizeof(tcPrint), "[AppMqtt] Connecting to MQTT broker: %s\r\n", tcURI);
        APP_TRACE(tcPrint);

        mqttClient.setURI(tcURI, stAppMqtt_Cfg.tcLogin, stAppMqtt_Cfg.tcPwd);
        mqttClient.setCaCert(CAcert);
        mqttClient.loopStart();
        bAppMqtt_ConfOnce = true;
    }
}

bool bAppMqtt_SyncConfig(void)
{
    memset(stAppMqtt_Cfg, 0, sizeof(TstAppMqtt_Config)); // wipe Mqtt config
    bAppCfg_LockJson();
    const char *pcId =      jAppCfg_Config["DEVICE_NAME"];
    const char *pcBroker =  jAppCfg_Config["MQTT"]["ADDR"];
    const char *pcLogin =   jAppCfg_Config["MQTT"]["LOGIN"];
    const char *pcPwd =     jAppCfg_Config["MQTT"]["PWD"];
    const char *pcTopic =   jAppCfg_Config["MQTT"]["GLOBAL_TOPIC"];
    uint16_t u16port =      jAppCfg_Config["MQTT"]["PORT"];
    uint16_t u16keepAlive = jAppCfg_Config["MQTT"]["KEEPALIVE"];
    if ((pcBroker == nullptr) || !strlen(pcBroker) || !u16port || pcTopic == nullptr || !strlen(pcTopic))
    {   // invalid minimal configuration
        stAppMqtt_Cfg.bAvailable = false;
    }
    else
    {
        stAppMqtt_Cfg.bAvailable = true;
        strncpy(stAppMqtt_Cfg.tcBroker, pcBroker, sizeof(stAppMqtt_Cfg.tcBroker)-1);
        strncpy(stAppMqtt_Cfg.tcPwd, pcTopic, sizeof(stAppMqtt_Cfg.tcPwd)-1);
        stAppMqtt_Cfg.u16Port = u16port;
        stAppMqtt_Cfg.u16KeepAlive = u16keepAlive;
        if ((pcLogin != nullptr) && strlen(pcLogin))
        {
            strncpy(stAppMqtt_Cfg.tcLogin, pcLogin, sizeof(stAppMqtt_Cfg.tcLogin)-1);
        }
        if ((pcPwd != nullptr) && strlen(pcPwd))
        {
            strncpy(stAppMqtt_Cfg.tcPwd, pcPwd, sizeof(stAppMqtt_Cfg.tcLogin)-1);
        }
    }
    bAppCfg_UnlockJson();
    return stAppMqtt_Cfg.bAvailable;
}

void onMqttConnect(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isConnected())
    {
        APP_TRACE("[AppMqtt] MQTT connected !\r\n");
    }
    if (mqttClient.isMyTurn(client))
    {
        vAppMqtt_Subscribe(tstTopicHandles, eAppMqtt_Topic_Nb);
    }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
    mqttClient.onEventCallback(event);
}

eApp_RetVal eAppMqtt_Publish(TeAppMqtt_Topics eTopicId, const char* pcPayload)
{
    eApp_RetVal eRet = eRet_Ok;
    TstAppMqtt_TopicHandle *pstHandle = &tstTopicHandles[eTopicId];
    if ((eTopicId >= eAppMqtt_Topic_Nb) || (pcPayload == nullptr))
    { _MNG_RETURN(eRet_BadParameter); }
    else{
       eRet = mqttClient.publish(std::string(pstHandle->tcTopic), std::string(pcPayload), 1) ? eRet_Ok : eRet_InternalError;
    }
    return eRet;
}

/*******************************************************************************
 *  Static
 ******************************************************************************/
static eApp_RetVal eAppMqtt_initTopics(void)
{
    static bool bInitOnce = false;
    eApp_RetVal eRet = eRet_Ok;
    if (!bInitOnce)
    {
        TstAppMqtt_TopicHandle *pstHandle = tstTopicHandles;
        MessageReceivedCallback *pCallbacks = tpfAppMqtt_Callbacks;
        for (TeAppMqtt_Topics eCnt = 0; eCnt < eAppMqtt_Topic_Nb; eCnt++)
        {
            if ((*pstHandle == nullptr) || (*pCallbacks == nullptr))
            { // avoid some runtime crash... Will probably never happen
                _MNG_RETURN(eRet_NullPointer);
                break;
            }
            else if (pstHandle->tcTopic[0] == '\0')
            { // already configured !
                _MNG_RETURN(eRet_Warning);
                break;
            }
            pstHandle->pfCallback = *pCallbacks;
            pstHandle->eId = eCnt; // That's very dirty, I know xD
            switch (pstHandle->eId)
            {
            case eAppMqtt_Topic_Event:
                strncpy(pstHandle->tcTopic, stAppMqtt_Cfg.tcTopic, MQTT_TOPIC_MAX_LENGTH - 1);
                break;

            case eAppMqtt_Topic_Cmd:
                snprintf(pstHandle->tcTopic, MQTT_TOPIC_MAX_LENGTH, "%s/%s", stAppMqtt_Cfg.tcTopic, stAppMqtt_Cfg.tcId);
                break;

            default:
                break;
            }
            pCallbacks++;
            pstHandle++;
        }
        if (eRet >= eRet_Ok)
        {
            bInitOnce = true;
        }
    }
    return eRet;
}

static void vAppMqtt_Subscribe(TstAppMqtt_TopicHandle *pstMqttHandle, size_t xSize)
{
    TstAppMqtt_TopicHandle *pstHandle = pstMqttHandle;
    for (size_t xCnt = 0; xCnt < xSize; xCnt++)
    {
        if (pstHandle->pfCallback != nullptr)
        {
            mqttClient.subscribe(pstHandle->tcTopic, pstHandle->pfCallback, 1);
        }
        pstHandle++;
    }
}

#endif

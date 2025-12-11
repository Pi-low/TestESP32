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
/*******************************************************************************
 *  Types, nums, macros
 ******************************************************************************/
#define MQTT_BUFFER_PARAM_LENGTH    64
typedef struct {
    bool bAvailable;
    TickType_t xTimeout;
    char tcId[MQTT_BUFFER_PARAM_LENGTH];
    char tcBroker[MQTT_BUFFER_PARAM_LENGTH];
    char tcLogin[MQTT_BUFFER_PARAM_LENGTH];
    char tcPwd[MQTT_BUFFER_PARAM_LENGTH];
    char tcTopic[MQTT_BUFFER_PARAM_LENGTH];
    uint16_t u16Port;
    uint16_t u16KeepAlive;
} TstAppMqtt_Config;

/*******************************************************************************
 *  Global variable
 ******************************************************************************/
ESP32MQTTClient mqttClient;
const char* CAcert = nullptr;
static TstAppMqtt_Config stAppMqtt_Cfg;
TstAppMqtt_TopicHandle stAppMqtt_TopicHandles[] = {
    {.eTopicType = eAppMqtt_SubTopic, .pfCallback = nullptr},  //eAppMqtt_Topic_EventIn
    {.eTopicType = eAppMqtt_PubTopic, .pfCallback = nullptr},   //eAppMqtt_Topic_EventOut
    {.eTopicType = eAppMqtt_SubTopic, .pfCallback = nullptr},   //eAppMqtt_Topic_Cmd
    {.eTopicType = eAppMqtt_PubTopic, .pfCallback = nullptr},   //eAppMqtt_Topic_Resp
    {.eTopicType = eAppMqtt_SubTopic, .pfCallback = nullptr},   //eAppMqtt_Topic_Substrip
};

/*******************************************************************************
 *  Prototypes
 ******************************************************************************/

/*******************************************************************************
 *  Functions
 ******************************************************************************/
void vAppMqtt_init(void)
{

}

void vAppMqtt_connect(void)
{
    char tcURI[128];
    char pcPrint[256];
    bAppMqtt_SyncConfig();
    if (stAppMqtt_Cfg.u16KeepAlive)
    {
        mqttClient.setKeepAlive(stAppMqtt_Cfg.u16KeepAlive);
    }
    if (stAppWifi_Config.pcHostName != nullptr)
    {
        mqttClient.setMqttClientName(stAppWifi_Config.tcId);
    }
    snprintf(tcURI, sizeof(tcURI), "mqtts://%s:%u", stAppMqtt_Cfg.tcBroker, stAppMqtt_Cfg.u16Port);
    snprintf(pcPrint, sizeof(pcPrint), "[AppWifi] Connecting to %s\r\n", pcURI);
    APP_TRACE(pcPrint);
    mqttClient.setURI(pcURI, stAppMqtt_Cfg.tcLogin, stAppMqtt_Cfg.tcPwd);
    mqttClient.setCaCert(CAcert);
    mqttClient.loopStart();
}

void vAppMqtt_subscribe(TstAppMqtt_TopicHandle *pstMqttHandle)
{

}

bool bAppMqtt_SyncConfig(void)
{
    bAppCfg_LockJson();
    const char *pcId =      jAppCfg_Config["DEVICE_NAME"];
    const char *pcBroker =  jAppCfg_Config["MQTT"]["ADDR"];
    const char *pcLogin =   jAppCfg_Config["MQTT"]["LOGIN"];
    const char *pcPwd =     jAppCfg_Config["MQTT"]["PWD"];
    const char *pcTopic =   jAppCfg_Config["MQTT"]["GLOBAL_TOPIC"];
    uint16_t u16port =      jAppCfg_Config["MQTT"]["PORT"];
    uint16_t u16keepAlive = jAppCfg_Config["MQTT"]["KEEPALIVE"];

    if ((pcBroker == nullptr) || !strlen(pcBroker) ||
        (pcId == nullptr) || !strlen(pcId) ||
        (pcTopic == nullptr) || !strlen(pcTopic) ||
        !u16port || !u16keepAlive)
    { // minimal valid configuration check
        stAppMqtt_Cfg.bAvailable = false;
    }
    else if ((strlen(pcBroker) >= sizeof(stAppMqtt_Cfg.tcBroker)) || (strlen(pcId) >= sizeof(stAppMqtt_Cfg.tcId)))
    {
        stAppMqtt_Cfg.bAvailable = false;
    }
    else
    {
        stAppMqtt_Cfg.bAvailable = true;

        strncpy(stAppMqtt_Cfg.tcBroker, pcBroker, sizeof(stAppMqtt_Cfg.tcBroker));
        strncpy(stAppMqtt_Cfg.tcId, pcId, sizeof(stAppMqtt_Cfg.tcId));
        strncpy(stAppMqtt_Cfg.tcTopic, pcTopic, sizeof(stAppMqtt_Cfg.tcTopic));
        if (pcLogin != nullptr)
        {
            strncpy(stAppMqtt_Cfg.tcLogin, pcLogin, sizeof(stAppMqtt_Cfg.tcLogin));
        }
        if (pcPwd != nullptr)
        {
            strncpy(stAppMqtt_Cfg.tcPwd, pcPwd, sizeof(stAppMqtt_Cfg.tcPwd));
        }
        stAppMqtt_Cfg.u16Port = u16port;
        stAppMqtt_Cfg.u16KeepAlive = u16keepAlive;
    }
    bAppCfg_UnlockJson();
    return stAppMqtt_Cfg.bAvailable;
}

void onMqttConnect(esp_mqtt_client_handle_t client)
{
    if (mqttClient.isConnected())
    {
        APP_TRACE("[AppWifi] MQTT connected !\r\n");
    }
    if (mqttClient.isMyTurn(client))
    {
        if (mqttClient.subscribe(std::string(APP_ROOT_TOPIC), MqttCallback_Main))
        {
        }
        mqttClient.subscribe(std::string(APP_ROOT_TOPIC) + "/" + std::string(stAppWifi_Config.pcHostName), MqttCallback_Device);
    }
}

void handleMQTT(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    auto *event = static_cast<esp_mqtt_event_handle_t>(event_data);
    mqttClient.onEventCallback(event);
}



#endif

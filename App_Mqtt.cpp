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
typedef struct {
    bool bAvailable;
    TickType_t xTimeout;
    const char* pcBroker;
    const char* pcLogin;
    const char* pcPwd;
    const char* pcTopic;
    uint16_t u16Port;
    uint16_t u16KeepAlive;
} TstAppWifi_MqttConfig;

/*******************************************************************************
 *  Global variable
 ******************************************************************************/
const char* CAcert = nullptr;
static TstAppWifi_MqttConfig stAppWifi_MqttCfg;

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
    if (stAppWifi_MqttCfg.u16KeepAlive)
    {
        mqttClient.setKeepAlive(stAppWifi_MqttCfg.u16KeepAlive);
    }
    if (stAppWifi_Config.pcHostName != nullptr)
    {
        mqttClient.setMqttClientName(stAppWifi_Config.pcHostName);
    }
    snprintf(pcURI, 64, "mqtts://%s:%u", stAppWifi_MqttCfg.pcBroker, stAppWifi_MqttCfg.u16Port);
    snprintf(pcPrint, 256, "[AppWifi] Connecting to MQTT broker: %s\r\n", pcURI);
    APP_TRACE(pcPrint);
    mqttClient.setURI(pcURI, stAppWifi_MqttCfg.pcLogin, stAppWifi_MqttCfg.pcPwd);
    mqttClient.setCaCert(CAcert);
    // mqttClient.setURL(stAppWifi_MqttCfg.pcBroker, stAppWifi_MqttCfg.u16Port, stAppWifi_MqttCfg.pcLogin, stAppWifi_MqttCfg.pcPwd);
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
    bAppCfg_UnlockJson();
    if ((pcBroker == nullptr) || !strlen(pcBroker) || !u16port || (pcId == nullptr )|| !strlen(pcId))
    {   // minimal valid configuration
        stAppWifi_MqttCfg.bAvailable = false;
    }
    else
    {
        stAppWifi_MqttCfg.bAvailable = true;
        stAppWifi_MqttCfg.pcBroker = pcBroker;
        stAppWifi_MqttCfg.pcLogin = pcLogin;
        stAppWifi_MqttCfg.pcPwd = pcPwd;
        stAppWifi_MqttCfg.pcTopic = pcTopic;
        stAppWifi_MqttCfg.u16Port = u16port;
        stAppWifi_MqttCfg.u16KeepAlive = u16keepAlive;
    }
    return stAppWifi_MqttCfg.bAvailable;
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

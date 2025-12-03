/**
 * @brief WiFi Application layer
 * @file App_Wifi.cpp
 * @version 0.1
 * @date 2025-12-02
 * @author your name (you@domain.com)
 */

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "App_Wifi.h"
#include <ArduinoMqttClient.h>

#if defined(APP_WIFI) && APP_WIFI
/*******************************************************************************
 *  Types, nums, macros
 ******************************************************************************/
#if APP_TASKS
#define WIFI_TASK            "APP_WIFI"
#define WIFI_TASK_HEAP       (configMINIMAL_STACK_SIZE*4)
#define WIFI_TASK_PARAM      NULL
#define WIFI_TASK_PRIO       2
#define WIFI_TASK_HANDLE     NULL
static void vAppWifi_Task(void *pvArg);
#endif

#define WIFI_ABORT_CONNECT 3

#define _MNG_RETURN(x)                      eRet = x

// Définition des états de la connexion WiFi
typedef enum {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
} TeAppWifi_State;


// Structure pour la configuration WiFi
typedef struct {
    const char* pcSsid;
    const char* pcPassword;
    const char* pcHostName;
    uint8_t u8Available;
} TstAppWifi_Config;

typedef struct {
    bool bAvailable;
    const char* pcBroker;
    uint16_t u16Port;
    uint16_t u16KeepAlive;
} TstAppWifi_MqttConfig;

/*******************************************************************************
 *  Global variable
 ******************************************************************************/
WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
static TstAppWifi_Config stAppWifi_Config;
static TeAppWifi_State eAppWifi_State = WIFI_DISCONNECTED;
static TstAppWifi_MqttConfig stAppWifi_MqttCfg = {false, nullptr, 0, 0};

/*******************************************************************************
 *  Prototypes
 ******************************************************************************/
static void vAppWifi_OnMqttMsg(int MsgSize);

/*******************************************************************************
 *  Functions
 ******************************************************************************/

/*******************************************************************************
 * @brief Initialize WiFi
 * 
 * @return eApp_RetVal 
 ******************************************************************************/
eApp_RetVal eAppWifi_init(void)
{
    eApp_RetVal eRet = eRet_Ok;
    // Extraire SSID et mot de passe depuis le JSON
    vAppWifi_GetWifiConfig();
    vAppWifi_GetMqttConfig();

#if APP_TASKS
    xTaskCreate(vAppWifi_Task, WIFI_TASK, WIFI_TASK_HEAP, WIFI_TASK_PARAM, WIFI_TASK_PRIO, WIFI_TASK_HANDLE);
#endif

    return eRet;
}

/*******************************************************************************
 * @brief WiFi Task
 * 
 * @param pvArg 
 ******************************************************************************/
static void vAppWifi_Task(void *pvArg)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(WIFI_TASKING); // Vérification toutes les 5 secondes
    uint8_t u8StopCounter = WIFI_ABORT_CONNECT;

    while (1)
    {
        switch (eAppWifi_State)
        {
        case WIFI_DISCONNECTED:
            // Tentative de connexion au WiFi
            vAppWifi_GetWifiConfig();
            if (stAppWifi_Config.u8Available)
            {
                APP_TRACE("Connecting to WiFi...\r\n");
                WiFi.mode(WIFI_STA);
                if (stAppWifi_Config.pcHostName != nullptr)
                {
                    WiFi.setHostname(stAppWifi_Config.pcHostName);
                    mqttClient.setId(stAppWifi_Config.pcHostName);
                }
                WiFi.begin(stAppWifi_Config.pcSsid, stAppWifi_Config.pcPassword);
                eAppWifi_State = WIFI_CONNECTING;
            }
            break;

        case WIFI_CONNECTING:
            // Vérification de l'état de la connexion
            if (WiFi.status() == WL_CONNECTED)
            {
                eAppWifi_State = WIFI_CONNECTED;
                APP_TRACE("WiFi connected !\r\n");
                if (stAppWifi_MqttCfg.bAvailable)
                {
                    if (mqttClient.connect(stAppWifi_MqttCfg.pcBroker, stAppWifi_MqttCfg.u16Port))
                    {
                        APP_TRACE("MQTT connected !\r\n");
                        mqttClient.subscribe("test/topic");
                        mqttClient.onMessage(vAppWifi_OnMqttMsg);
                    }
                    else
                    {
                        APP_TRACE("MQTT connection failed !\r\n");
                    }
                }
            }
            else
            {
                // Échec de connexion, retour à l'état déconnecté
                APP_TRACE("Fail to connect, next attempt in 5000 ms...\r\n");
                u8StopCounter--;
                if (!u8StopCounter) {
                    APP_TRACE("Abort operation, clear wifi settings...\r\n");
                    bAppCfg_LockJson();
                    jAppCfg_Config["WIFI"]["SSID"] = nullptr;
                    jAppCfg_Config["WIFI"]["PWD"] = nullptr;
                    stAppWifi_Config.u8Available = 0;
                    bAppCfg_UnlockJson();
                    u8StopCounter = WIFI_ABORT_CONNECT;
                }
                eAppWifi_State = WIFI_DISCONNECTED;
            }
            break;  

        case WIFI_CONNECTED:
            // Vérification périodique de la connexion
            mqttClient.poll();
            if (WiFi.status() != WL_CONNECTED)
            {
                eAppWifi_State = WIFI_DISCONNECTED;
                APP_TRACE("WiFi disconnected, attempting to reconnect...\r\n");
            }
            break;

        default:
            // État inconnu, réinitialisation à déconnecté
            eAppWifi_State = WIFI_DISCONNECTED;
            APP_TRACE("Unknown WiFi state\r\n");
            break;
        }

        // Synchronisation avec la période
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void vAppWifi_OnMqttMsg(int MsgSize)
{
    static char tcBuffer[128];
    APP_TRACE("MQTT Rx: ");
    memset(tcBuffer, 0, 128);
    if (mqttClient.available())
    {
        mqttClient.readBytes(tcBuffer, MIN(MsgSize, sizeof(tcBuffer) - 1));
        APP_TRACE(tcBuffer);
    }
    APP_TRACE("\r\n");
}

void vAppWifi_GetWifiConfig(void)
{
    bAppCfg_LockJson();
    const char *ssid = jAppCfg_Config["WIFI"]["SSID"];
    const char *pwd = jAppCfg_Config["WIFI"]["PWD"];
    stAppWifi_Config.pcHostName = jAppCfg_Config["DEVICE_NAME"];
    if (ssid == nullptr || pwd == nullptr || strlen(ssid) == 0 || strlen(pwd) == 0)
    {
        stAppWifi_Config.u8Available = 0;
    }
    else
    {
        stAppWifi_Config.u8Available = 1;
        stAppWifi_Config.pcSsid = ssid;
        stAppWifi_Config.pcPassword = pwd;
    }
    bAppCfg_UnlockJson();
}

void vAppWifi_GetMqttConfig(void)
{
    bAppCfg_LockJson();
    const char *broker = jAppCfg_Config["MQTT"]["ADDR"];
    uint16_t port = jAppCfg_Config["MQTT"]["PORT"];
    uint16_t keepAlive = jAppCfg_Config["MQTT"]["KEEPALIVE"];
    if (broker == nullptr || port == 0 || keepAlive == 0)
    {
        stAppWifi_MqttCfg.bAvailable = 0;
    }
    else
    {
        stAppWifi_MqttCfg.bAvailable = 1;
        stAppWifi_MqttCfg.pcBroker = broker;
        stAppWifi_MqttCfg.u16Port = port;
        stAppWifi_MqttCfg.u16KeepAlive = keepAlive;
    }
    bAppCfg_UnlockJson();
}

#endif // APP_WIFI

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
#include "ESP32MQTTClient.h"
#include "time.h"

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

#define MAX_RETRY 3
#define LOCAL_PRINT_BUFFER  256
#define CONNECT_TIMEOUT     5000

// Définition des états de la connexion WiFi
typedef enum {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED,
} TeAppWifi_State;

// Structure pour la configuration WiFi
typedef struct {
    bool bAvailable;
    TickType_t xTimeout;
    char tcSsid[CFG_PARAM_MAX_LEN];
    char tcPassword[CFG_PARAM_MAX_LEN];
    char tcHostName[CFG_PARAM_MAX_LEN];
} TstAppWifi_Config;

/*******************************************************************************
 *  Global variable
 ******************************************************************************/
ESP32MQTTClient mqttClient;
static bool bAppWifi_DisableWifi;
static TstAppWifi_Config stAppWifi_Cfg;
static TeAppWifi_State eAppWifi_State = WIFI_DISCONNECTED;

/*******************************************************************************
 *  Prototypes
 ******************************************************************************/
void vAppWifi_OnWifiConnect(void);
// void MqttCallback_Main(const std::string payload);
// void MqttCallback_Device(const std::string payload);

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
    bAppWifi_DisableWifi = false;
    stAppWifi_Cfg.bAvailable = false;
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
    const TickType_t xFrequency = pdMS_TO_TICKS(WIFI_TASKING);
    char tcPrint[LOCAL_PRINT_BUFFER];
    while (1)
    {
        switch (eAppWifi_State)
        {
        case WIFI_DISCONNECTED:
            if (bAppWifi_SyncWifiConfig() && !bAppWifi_DisableWifi)
            {
                snprintf(tcPrint, sizeof(tcPrint), "[AppWifi] Connectig to %s\r\n", stAppWifi_Cfg.pcSsid);
                APP_TRACE(tcPrint);
                WiFi.mode(WIFI_STA);
                if (stAppWifi_Cfg.tcHostName[0] != '\0')
                { WiFi.setHostname(stAppWifi_Cfg.tcHostName); }

                WiFi.begin(stAppWifi_Cfg.tcSsid, stAppWifi_Cfg.tcPassword);
                stAppWifi_Cfg.xTimeout = xTaskGetTickCount() + CONNECT_TIMEOUT;
                eAppWifi_State = WIFI_CONNECTING;
            }
            break;

        case WIFI_CONNECTING:
            if (WiFi.status() == WL_CONNECTED)
            {
                eAppWifi_State = WIFI_CONNECTED;
                APP_TRACE("\r\n[AppWifi] WiFi connected !\r\n");
                vAppWifi_OnWifiConnect();
            }
            else if (stAppWifi_Cfg.xTimeout > xTaskGetTickCount())
            {   // connexion ongoing ...
                APP_TRACE(".");
            }
            else
            {
                APP_TRACE("\r\n[AppWifi] Connexion timeout !...\r\n");
                bAppWifi_DisableWifi = true;
                eAppWifi_State = WIFI_DISCONNECTED;
            }
            break;  

        case WIFI_CONNECTED:
            // check connexion state
            if (WiFi.status() != WL_CONNECTED)
            {
                eAppWifi_State = WIFI_DISCONNECTED;
                APP_TRACE("[AppWifi] WiFi disconnected, attempting to reconnect...\r\n");
            }
            break;

        default:
            // Unknown state
            eAppWifi_State = WIFI_DISCONNECTED;
            APP_TRACE("[AppWifi] Unknown WiFi state\r\n");
            break;
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void vAppWifi_OnWifiConnect(void)
{
    configTime(3600, 0, "pool.ntp.org");
}

// void MqttCallback_Main(const std::string payload)
// {
//     char tcBuffer[256];
//     snprintf(tcBuffer, 256, "[AppWifi] Mqtt rx from \"/lumiapp\" -> %s\r\n", payload.c_str());
//     APP_TRACE(tcBuffer);
// }

// void MqttCallback_Device(const std::string payload)
// {
//     char tcBuffer[256];
//     snprintf(tcBuffer, 256, "[AppWifi] Mqtt rx from \"/lumiapp/%s\" -> %s\r\n", stAppWifi_Cfg.pcHostName, payload.c_str());
//     APP_TRACE(tcBuffer);
// }

bool bAppWifi_SyncWifiConfig(void)
{
    memset(stAppWifi_Cfg, 0, sizeof(stAppWifi_Cfg)); // wipe config struct
    bAppCfg_LockJson();
    const char *pcSsid = jAppCfg_Config["WIFI"]["SSID"];
    const char *pcPwd = jAppCfg_Config["WIFI"]["PWD"];
    const char *pcHostname = jAppCfg_Config["DEVICE_NAME"];
    if ((pcSsid == nullptr) || (pcPwd == nullptr) || !strlen(pcSsid) || !strlen(pcPwd))
    {   // invalid minimal configuration
        stAppWifi_Cfg.bAvailable = false;
    }
    else
    {
        stAppWifi_Cfg.bAvailable = true;
        strncpy(stAppWifi_Cfg.tcPassword, pcPwd, sizeof(stAppWifi_Cfg.tcPassword)-1);
        strncpy(stAppWifi_Cfg.tcSsid, pcPwd, sizeof(stAppWifi_Cfg.tcSsid)-1);
        if ((pcHostname != nullptr) && strlen(pcHostname))
        {
            strncpy(stAppWifi_Cfg.tcHostName, pcHostname, sizeof(stAppWifi_Cfg.tcHostName)-1);
        }
    }
    bAppCfg_UnlockJson();
    return stAppWifi_Cfg.bAvailable;
}

#endif // APP_WIFI

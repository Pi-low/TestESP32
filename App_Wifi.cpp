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
#define LOCAL_PRINT_BUFFER          256
#define CONNECT_TIMEOUT_MS          30000
#define RETRY_TIMEOUT_MS            300000
#define WIFI_BUFFER_PARAM_LENGTH    64

#define _MNG_RETURN(x)                      eRet = x

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
    char tcSsid[WIFI_BUFFER_PARAM_LENGTH];
    char tcPassword[WIFI_BUFFER_PARAM_LENGTH];
    char tcHostName[WIFI_BUFFER_PARAM_LENGTH];
} TstAppWifi_Config;

/*******************************************************************************
 *  Global variable
 ******************************************************************************/
static bool bAppWifi_DisableWifi;
static TstAppWifi_Config stAppWifi_Config;
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
    stAppWifi_Config.bAvailable = false;
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
    char pcPrintBuffer[LOCAL_PRINT_BUFFER];
    while (1)
    {
        switch (eAppWifi_State)
        {
        case WIFI_DISCONNECTED:
            if (bAppWifi_SyncWifiConfig() && !bAppWifi_DisableWifi)
            {
                snprintf(pcPrintBuffer, LOCAL_PRINT_BUFFER, "[AppWifi] Connectig to %s\r\n", stAppWifi_Config.tcSsid);
                APP_TRACE(pcPrintBuffer);
                WiFi.mode(WIFI_STA);
                WiFi.setHostname(stAppWifi_Config.tcHostName); }
                WiFi.begin(stAppWifi_Config.tcSsid, stAppWifi_Config.tcPassword);
                stAppWifi_Config.xTimeout = xTaskGetTickCount() + CONNECT_TIMEOUT_MS;
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
            else if (stAppWifi_Config.xTimeout > xTaskGetTickCount())
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
            // Vérification périodique de la connexion
            if (WiFi.status() != WL_CONNECTED)
            {
                eAppWifi_State = WIFI_DISCONNECTED;
                APP_TRACE("[AppWifi] WiFi disconnected, attempting to reconnect...\r\n");
            }
            break;

        default:
            // État inconnu, réinitialisation à déconnecté
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

void MqttCallback_Main(const std::string payload)
{
    char tcBuffer[256];
    snprintf(tcBuffer, 256, "[AppWifi] Mqtt rx from \"/lumiapp\" -> %s\r\n", payload.c_str());
    APP_TRACE(tcBuffer);
}

void MqttCallback_Device(const std::string payload)
{
    char tcBuffer[256];
    snprintf(tcBuffer, 256, "[AppWifi] Mqtt rx from \"/lumiapp/%s\" -> %s\r\n", stAppWifi_Config.pcHostName, payload.c_str());
    APP_TRACE(tcBuffer);
}

bool bAppWifi_SyncWifiConfig(void)
{
    memset(stAppWifi_Config.tcHostName, 0, sizeof(stAppWifi_Config.tcHostName));
    memset(stAppWifi_Config.tcSsid, 0, sizeof(stAppWifi_Config.tcSsid));
    memset(stAppWifi_Config.tcPassword, 0, sizeof(stAppWifi_Config.tcPassword));

    bAppCfg_LockJson();
    const char *pcSsid = jAppCfg_Config["WIFI"]["SSID"];
    const char *pcPwd = jAppCfg_Config["WIFI"]["PWD"];
    const char *pcHostname = jAppCfg_Config["DEVICE_NAME"];
    // check data valid 
    if ((pcSsid == nullptr) || !strlen(pcSsid) ||
        (pcPwd == nullptr) || !strlen(pcPwd) ||
        (pcHostname == nullptr) || !strlen(pcHostname))
    {
        // empty data
        stAppWifi_Config.bAvailable = false;
    }
    else if ((strlen(pcSsid) >= sizeof(stAppWifi_Config.tcSsid)) ||
             (strlen(pcPwd) >= sizeof(stAppWifi_Config.tcPassword)) ||
             (strlen(pcHostname) >= sizeof(stAppWifi_Config.tcHostName)))
    {
        // Data exceed destination buffer
        stAppWifi_Config.bAvailable = false;
    }
    else
    {
        stAppWifi_Config.bAvailable = true;
        strncpy(stAppWifi_Config.tcHostName, pcHostname, sizeof(stAppWifi_Config.tcHostName)-1);
        strncpy(stAppWifi_Config.tcSsid, pcSsid, sizeof(stAppWifi_Config.tcSsid)-1);
        strncpy(stAppWifi_Config.tcPassword, pcPwd, sizeof(stAppWifi_Config.tcPassword)-1);
    }
    bAppCfg_UnlockJson();
    return stAppWifi_Config.bAvailable;
}

#endif // APP_WIFI

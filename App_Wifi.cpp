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

#if defined(APP_WIFI) && APP_WIFI
/*******************************************************************************
 *  Types, nums, macros
 ******************************************************************************/
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
    uint8_t u8Available;
} TstAppWifi_Config;

/*******************************************************************************
 *  Global variable
 ******************************************************************************/
static TstAppWifi_Config stAppWifi_Config;
static TeAppWifi_State eAppWifi_State = WIFI_DISCONNECTED;

/*******************************************************************************
 *  Prototypes
 ******************************************************************************/
static void vAppWifi_Task(void *pvArg);

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
    vappWifi_GetWifiConfig();

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

    while (1)
    {
        switch (eAppWifi_State)
        {
        case WIFI_DISCONNECTED:
            // Tentative de connexion au WiFi
            if (stAppWifi_Config.u8Available)
            {
                APP_TRACE("Connecting to WiFi...\r\n");
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
                char ipMsg[32];
                snprintf(ipMsg, sizeof(ipMsg), "IP address: %s\r\n", WiFi.localIP().toString().c_str());
                APP_TRACE(ipMsg);
            }
            else
            {
                // Échec de connexion, retour à l'état déconnecté
                APP_TRACE("Fail to connect, next attempt in 5000 ms...\r\n");
                eAppWifi_State = WIFI_DISCONNECTED;
            }
            break;

        case WIFI_CONNECTED:
            // Vérification périodique de la connexion
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

void vappWifi_GetWifiConfig(void)
{
    bAppCfg_LockJson();
    const char *ssid = jAppCfg_Config["WIFI"]["SSID"];
    const char *pwd = jAppCfg_Config["WIFI"]["PWD"];
    bAppCfg_UnlockJson();
    if (ssid == nullptr || pwd == nullptr || strlen(ssid) == 0 || strlen(pwd) == 0)
    {
        stAppWifi_Config.u8Available = 0;
        APP_TRACE("Wifi config unavailable\r\n");
    }
    else
    {
        stAppWifi_Config.u8Available = 1;
        stAppWifi_Config.pcSsid = ssid;
        stAppWifi_Config.pcPassword = pwd;
    }
}

#endif // APP_WIFI

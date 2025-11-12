

/**
 * @file Config.cpp
 * @brief Application configuration implementation
 * @version 0.1
 * @date 2025-11-11
 * @author Nello
 */

#include "App_PrintUtils.h"
#include "Config.h"
#include "FS.h"
#include "FFat.h"
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#define CONFIG_FILE_PATH    "/config.cfg"

#define GEN_STRING(VALUE)                   #VALUE,

#define FOREACH_CONFIG_OBJ(PARAM)           \
    PARAM(DEVICE_NAME)                      \
    PARAM(WIFI)                             \
    PARAM(MQTT)                             \
    PARAM(TOKEN)                            \
    PARAM(DEVICE_SUBSTRIPS)                 \
    PARAM(DEVICE_PALETTES)                  \
    PARAM(DEVICE_PROG_ANIM)                 \
    PARAM(DEVICE_WORKING_TIMESLOTS)

static String strCfgObjects[] = {FOREACH_CONFIG_OBJ(GEN_STRING)};

JsonDocument<2048> jdocConfig;

eApp_RetVal eAppConfig_init(void) {
    bool bFret;
    bool bDefaultConfig = false;
    char tcWrBuffer[128];
    uint8_t i = 0;
    eApp_RetVal eRet = eRet_Ok;

    while (((bFret = FFat.begin()) == false) && (i < 1)) {
        FFat.format();
        snprintf(tcWrBuffer, 128, "Failed to mount FFat, formatting (%d)...\r\n", i);
        APP_TRACE(tcWrBuffer);
        i++;
    }

    snprintf(tcWrBuffer, 128, "Mount FFat: %s\r\n", bFret ? "success" : "fail");
    APP_TRACE(tcWrBuffer);

    if (bFret)
    {
        // File xConfigFile = FFat.open(CONFIG_FILE_PATH, FILE_READ, false);
        // if (xConfigFile) {
        // }
        // if (!xConfigFile) {
        //     eRet = eRet_Error;
        // }
        // else {
        // }
    }
    
        
    return eRet;
}

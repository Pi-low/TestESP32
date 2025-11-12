

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

eApp_RetVal eAppConfig_init(void) {
    bool bFret;
    uint8_t i = 0;
    eApp_RetVal eRet = eRet_Ok;

    while (((bFret = FFat.begin()) == false) && (i < 2)) {
        FFat.format();
        APP_TRACE("Failed to mount FFat, formatting...\r\n");
        i++;
    }

    if (bFret)
    {
        // File xConfigFile = FFat.open(CONFIG_FILE_PATH, "r");
        // if (!xConfigFile) {
        //     eRet = eRet_Error;
        // }
        // else {
        
        // }
    }
    
        
    return eRet;
}

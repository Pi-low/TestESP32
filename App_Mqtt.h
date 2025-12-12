/**
 * @brief MQTT Application header
 * @file App_MQTT.h
 * @version 0.1
 * @date 2025-12-11
 * @author Nello
 */

#ifndef _APP_MQTT_H_
#define _APP_MQTT_H_

/*******************************************************************************
 *  Includes
 ******************************************************************************/
#include "Config.h"

#if defined(APP_MQTT) && APP_MQTT
#include "ESP32MQTTClient.h"
/*******************************************************************************
 *  Types, nums, macros
 ******************************************************************************/
typedef enum {
    eAppMqtt_Topic_Event,
    eAppMqtt_Topic_Cmd,
    eAppMqtt_Topic_Nb
} TeAppMqtt_Topics;

/*******************************************************************************
 *  Global variable
 ******************************************************************************/

/*******************************************************************************
 *  Prototypes
 ******************************************************************************/
void vAppMqtt_init(void);
void vAppMqtt_connect(void);
bool bAppMqtt_SyncConfig(void);
eApp_RetVal eAppMqtt_Publish(TeAppMqtt_Topics eTopicId, const char* pcPayload);

#endif
#endif // _APP_MQTT_H_

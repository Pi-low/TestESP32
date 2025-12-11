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
typedef void (*pfAppMqtt_Callback) (const char* pcPayload, uint32_t u32PayloadLen);

typedef enum {
    eAppMqtt_SubTopic,
    eAppMqtt_PubTopic,
} TeAppMqtt_Type;

typedef enum {
    eAppMqtt_Topic_EventIn,
    eAppMqtt_Topic_EventOut,
    eAppMqtt_Topic_Cmd,
    eAppMqtt_Topic_Resp,
    eAppMqtt_Topic_Substrip,
} TeAppMqtt_Id;

typedef struct {
    TeAppMqtt_Type      eTopicType;
    QueueHandle_t       xQueueTopic;
    const char          *pcTopicName;
    pfAppMqtt_Callback  pfCallback;
} TstAppMqtt_TopicHandle;

/*******************************************************************************
 *  Global variable
 ******************************************************************************/

/*******************************************************************************
 *  Prototypes
 ******************************************************************************/
void vAppMqtt_init(void);
void vAppMqtt_connect(void);
bool bAppMqtt_SyncConfig(void);

#endif
#endif // _APP_MQTT_H_

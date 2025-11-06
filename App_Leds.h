/**
 * @brief WS2812 management config
 * @file Lights.h
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */

#ifndef _APP_LEDS_H
#define _APP_LEDS_H

#include "Config.h"

#if defined(APP_FASTLED) && APP_FASTLED

#include <FastLED.h>

#define LED_SUBSTRIP_LEN    20
#define LED_SUBSTRIP_NB     5

void AppLED_init(void);
void AppLED_showLoop(void);

#if APP_TASKS
extern SemaphoreHandle_t    xLedStripSema;
#define LOCK_LEDS()         xSemaphoreTake(xLedStripSema, portMAX_DELAY)
#define UNLOCK_LEDS()       xSemaphoreGive(xLedStripSema)
#endif

bool bAppLed_blackout(void);
bool bAppLed_resume(void);

#endif // APP_FASTLED

#endif //_APP_LEDS_H

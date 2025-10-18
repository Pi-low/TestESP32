/**
 * @brief WS2812 management config
 * @file Lights.h
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */

#ifndef _LIGHTS_H
#define _LIGHTS_H

#include "Config.h"

#if defined(APP_FASTLED) && APP_FASTLED

#include <FastLED.h>

#define LED_NB			100
#define LED_DATA_PIN	4
#define LED_CHIPSET		WS2812
#define LED_PIXEL_ORDER	GRB
#define LED_BRIGHTNESS	127
#define LED_REFRESH		20 //ms

void AppLED_init(void);
void AppLED_fillGauge(int CurrentVal, int MinVal, int MaxVal);
void AppLED_showLoop(void);

#endif // APP_FASTLED

#endif //_LIGHTS_H

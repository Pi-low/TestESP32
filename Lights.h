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
#define LED_BRIGHTNESS	128
#define LED_REFRESH		33 //ms

void AppLED_init(void);
void AppLED_fillGauge(int CurrentVal, int MinVal, int MaxVal);
void AppLED_setupBlink(uint8_t u8Pos, CRGB Color, uint16_t u16Period);
void AppLED_blink(uint8_t u8Enable);
void AppLED_showLoop(void);

/* Pirvate types, enum, definitions */
#ifdef _LIGHTS_C
typedef struct {
    CRGB Color;
    uint8_t u8State;
    uint8_t u8Pos;
    uint16_t u16Period;
} TsAppLED_BlinkAnim;
#endif

#endif // APP_FASTLED

#endif //_LIGHTS_H

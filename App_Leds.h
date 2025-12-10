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
#include "SubStrip.h"
#include "App_Cli.h"

#define LED_SUBSTRIP_LEN    20
#define LED_SUBSTRIP_NB     5
#define _LED_ALLSTRIPS      ((uint8_t)0xFF)

void AppLED_init(void);
void AppLED_showLoop(void);

#if APP_TASKS
extern SemaphoreHandle_t    xLedStripSema;
#define LOCK_LEDS()         xSemaphoreTake(xLedStripSema, portMAX_DELAY)
#define UNLOCK_LEDS()       xSemaphoreGive(xLedStripSema)
#endif

extern const char *tpcAppLED_Animations[];

eApp_RetVal eAppLed_blackout(void);
eApp_RetVal eAppLed_resume(void);
eApp_RetVal eAppLed_SetBrightness(uint8_t u8Value);
eApp_RetVal eAppLed_SetAnimation(SubStrip::TeAnimation eAnimation, uint8_t u8Index);
eApp_RetVal eAppLed_SetSpeed(uint8_t u8Speed, uint8_t u8Index);
eApp_RetVal eAppLed_SetPeriod(uint32_t u32Period, uint8_t u8Index);
eApp_RetVal eAppLed_SetFade(uint16_t u16FadeMs, uint8_t u8Index);
eApp_RetVal eAppLed_SetDirection(SubStrip::TeDirection eDirection, uint8_t u8Index);
eApp_RetVal eAppLed_SetOffset(uint8_t u8Offset, uint8_t u8Index);
eApp_RetVal eAppLed_SetBpm(uint8_t u8Bpm, uint8_t u8Index);
eApp_RetVal eAppLed_SetPalette(uint8_t u8PaletteIndex, uint8_t u8SubStripIndex);
eApp_RetVal eAppLed_LoadColorAt(CRGB xColor, uint8_t u8PaletteIndex, uint8_t u8Index);
eApp_RetVal eAppLed_LoadColors(CRGB *xColor, uint8_t u8NbColors, uint8_t u8PaletteIndex);
eApp_RetVal eAppLed_ConfigSubstrip(uint8_t u8StripId, uint8_t u8CmdIndex, const char* pcValue);

#endif // APP_FASTLED

#endif //_APP_LEDS_H

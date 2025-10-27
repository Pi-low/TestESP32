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

#if APP_TASKS
#define LED_TASK            "APP_LEDS"
#define LED_TASK_HEAP       2048
#define LED_TASK_PARAM      NULL
#define LED_TASK_PRIO       2
#define LED_TASK_HANDLE     NULL
#endif

#define LED_SUBSTRIP_LEN    20
#define LED_SUBSTRIP_NB     5
#define LED_DATA_PIN        4
#define LED_CHIPSET         WS2812
#define LED_PIXEL_ORDER     GRB
#define LED_BRIGHTNESS      127
#define LED_REFRESH         SUBSTRIP_FPS // use constant from SubStrip

#define _LED_TIMEOUT        _SUBSTRIP_PERIOD //ms
#define _LED_NB             (LED_SUBSTRIP_LEN * LED_SUBSTRIP_NB)
#define _LED_SUB_OFFSET(x)  (x * LED_SUBSTRIP_LEN)

void AppLED_init(void);
void AppLED_showLoop(void);

#endif // APP_FASTLED

#endif //_APP_LEDS_H

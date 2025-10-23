/**
 * @brief WS2812 management
 * @file Lights.c
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */
#define _LIGHTS_C
#include "App_Leds.h"

#if defined(APP_FASTLED) && APP_FASTLED

#include "SubStrip.h"

static CRGB ledStrip[_LED_NB];
static SubStrip SubStrips[LED_SUBSTRIP_NB] = {
    SubStrip(20, ledStrip + _SUB_OFFSET(0)),
    SubStrip(20, ledStrip + _SUB_OFFSET(1)),
    SubStrip(20, ledStrip + _SUB_OFFSET(2)),
    SubStrip(20, ledStrip + _SUB_OFFSET(3)),
    SubStrip(20, ledStrip + _SUB_OFFSET(4)),
};

CRGB pMyColorPalette1[3] = {CRGB::White, CRGB::Red, CRGB::Black};
CRGB pMyColorPalette2[3] = {CRGB::Orange, CRGB::Fuchsia, CRGB::Black};

/**
 * @brief Initialize ledstrip
 * 
 */
void AppLED_init(void) {
    FastLED.addLeds<LED_CHIPSET, LED_DATA_PIN, LED_PIXEL_ORDER>(ledStrip, _LED_NB);
	FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.clear();
    FastLED.show();
    for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
        SubStrips[i].vSetAnimation(SubStrip::CHECKERED, pMyColorPalette1, 2000, 3);
    }
}

/**
 * @brief Fill ledstrip as gauge
 * 
 * @param CurrentVal
 * @param MaxVal
 * @param Color
 */
void AppLED_fillGauge(int CurrentVal, int MinVal, int MaxVal) {
    uint8_t u8MappedOut = map(CurrentVal, MinVal, MaxVal, 0, _LED_NB);
    FastLED.clear();
    fill_gradient_RGB(ledStrip, _LED_NB, CRGB::Red, CRGB::Blue);
    fill_solid(ledStrip + (_LED_NB - u8MappedOut), u8MappedOut, CRGB::Black);
}

/**
 * @brief Selftimed ledstrip frefresh
 * 
 */
void AppLED_showLoop(void) {
    static uint32_t u32Timeout = 0;
    static uint32_t u32ColorFlipTimeout = 0;
    static uint32_t u32ShiftedTimeout = 0;
    uint32_t u32RightNow = millis();
    static uint8_t u8Toggle = 0;
    CRGB* Colors;

    if (u32ColorFlipTimeout < u32RightNow) {
        u32ColorFlipTimeout = u32RightNow + 2000;
        u8Toggle ^= 1;
        Colors = u8Toggle ? pMyColorPalette1 : pMyColorPalette2;
         for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
            SubStrips[i].vSetColorPalette(Colors);
        }
    }

    if (u32Timeout < u32RightNow) {
        u32Timeout = u32RightNow + _LED_TIMEOUT;
        for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
            SubStrips[i].vManageAnimation(u32RightNow);
        }
        FastLED.show();
    }
}

#endif // APP_FASTLED

/**
 * @brief WS2812 management
 * @file Lights.c
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */
#define _LIGHTS_C
#include "Lights.h"

#if defined(APP_FASTLED) && APP_FASTLED

#include "SubStrip.h"

static CRGB ledStrip[_LED_NB];
static SubStrip[LED_SUBSTRIP_NB];

SubStrip subStrip1(20, NULL);
SubStrip subStrip2(20, NULL);
SubStrip subStrip3(20, NULL);
SubStrip subStrip4(20, NULL);
SubStrip subStrip5(20, NULL);

CRGB pMyColorPalette1[3] = {CRGB::White, CRGB::Red, CRGB::Black};
CRGB pMyColorPalette2[3] = {CRGB::Orange, CRGB::Fuchsia, CRGB::Black};
/**
 * @brief Initialize ledstrip
 * 
 */
void AppLED_init(void) {
    FastLED.addLeds<LED_CHIPSET, LED_DATA_PIN, LED_PIXEL_ORDER>(ledStrip, LED_NB);
	FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.clear();
    FastLED.show();

    subStrip1.vSetAnimation(SubStrip::RAINDROPS, pMyColorPalette1, 2000, 1);
    subStrip2.vSetAnimation(SubStrip::CHECKERED, pMyColorPalette1, 1000, 6);
    subStrip3.vSetAnimation(SubStrip::RAINDROPS, pMyColorPalette1, 1500, 2);
    subStrip4.vSetAnimation(SubStrip::CHECKERED, pMyColorPalette2, 1000, 2);
    subStrip5.vSetAnimation(SubStrip::RAINDROPS, pMyColorPalette1, 2500, 2);
}

/**
 * @brief Fill ledstrip as gauge
 * 
 * @param CurrentVal
 * @param MaxVal
 * @param Color
 */
void AppLED_fillGauge(int CurrentVal, int MinVal, int MaxVal) {
    uint8_t u8MappedOut = map(CurrentVal, MinVal, MaxVal, 0, LED_NB);
    FastLED.clear();
    fill_gradient_RGB(ledStrip, LED_NB, CRGB::Red, CRGB::Blue);
    fill_solid(ledStrip + (LED_NB - u8MappedOut), u8MappedOut, CRGB::Black);
}

/**
 * @brief Selftimed ledstrip frefresh
 * 
 */
void AppLED_showLoop(void) {
    static uint32_t u32Timeout = 0;
    static uint32_t u32ShiftedTimeout = 0;
    uint32_t u32RightNow = millis();

    if (u32Timeout < u32RightNow) {
        u32Timeout = u32RightNow + _LED_TIMEOUT;

        subStrip1.vManageAnimation(u32RightNow);
        subStrip2.vManageAnimation(u32RightNow);
        subStrip3.vManageAnimation(u32RightNow);
        subStrip4.vManageAnimation(u32RightNow);
        subStrip5.vManageAnimation(u32RightNow);

        // mapping main led strip
        subStrip1.vGetSubStrip(ledStrip, 20);
        subStrip2.vGetSubStrip(ledStrip + 20, 20);
        subStrip3.vGetSubStrip(ledStrip + 40, 20);
        subStrip4.vGetSubStrip(ledStrip + 60, 20);
        subStrip5.vGetSubStrip(ledStrip + 80, 20);
        FastLED.show();
    }
}

#endif // APP_FASTLED

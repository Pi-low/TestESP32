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

static CRGB ledStrip[LED_NB];
static TsAppLED_BlinkAnim tsAppLED_Blink;

SubStrip subStrip1(20);
SubStrip subStrip2(20);
SubStrip subStrip3(20);
SubStrip subStrip4(20);
SubStrip subStrip5(20);
/**
 * @brief Initialize ledstrip
 * 
 */
void AppLED_init(void) {
    CRGB pMyColorPalette1[3] = {CRGB::White, CRGB::Red, CRGB::Black};
    CRGB pMyColorPalette2[3] = {CRGB::Orange, CRGB::Fuchsia, CRGB::Black};

    FastLED.addLeds<LED_CHIPSET, LED_DATA_PIN, LED_PIXEL_ORDER>(ledStrip, LED_NB);
	FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.setCorrection(TypicalLEDStrip);
    tsAppLED_Blink = (TsAppLED_BlinkAnim) {CRGB::Red, 0, 0, 200};
    FastLED.clear();
    FastLED.show();

    subStrip2.vSetColorPalette(pMyColorPalette1);
    subStrip4.vSetColorPalette(pMyColorPalette2);

    subStrip1.vSetAnimation(SubStrip::RAINDROPS, 2000, 1);;
    subStrip2.vSetAnimation(SubStrip::CHECKERED, 1000, 6, pMyColorPalette1);
    subStrip3.vSetAnimation(SubStrip::RAINDROPS, 1500, 2);
    subStrip4.vSetAnimation(SubStrip::CHECKERED, 1000, 2, pMyColorPalette2);
    subStrip5.vSetAnimation(SubStrip::RAINDROPS, 2500, 3);
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
    // fill_solid(ledStrip, u8MappedOut, Color);
    fill_gradient_RGB(ledStrip, LED_NB, CRGB::Red, CRGB::Blue);
    fill_solid(ledStrip + (LED_NB - u8MappedOut), u8MappedOut, CRGB::Black);
}

/**
 * @brief Toggle one led every call
 * 
 * @param u8Pos 
 * @param Color 
 * @param u16Period periodicity, (uint16_t)(-1) = disable
 */
void AppLED_setupBlink(uint8_t u8Pos, CRGB Color, uint16_t u16Period) {
        tsAppLED_Blink.u16Period = u16Period;
        tsAppLED_Blink.Color = Color;
        tsAppLED_Blink.u8Pos = u8Pos;
}

/**
 * @brief Enable/disable led blink anim
 * 
 * @param u8Enable
 */
void AppLED_blink(uint8_t u8Enable) {
    tsAppLED_Blink.u8State = u8Enable;
}

/**
 * @brief Selftimed ledstrip frefresh
 * 
 */
void AppLED_showLoop(void) {
    static uint32_t u32Timeout = 0;
    static uint32_t u32ShiftedTimeout = 0;
    uint32_t u32RightNow = millis();
    uint32_t u32NowShifted = u32RightNow + LED_REFRESH;

    // static uint32_t u32BlinkTmout = 0;
    // static uint8_t u8Toggle = 0;
    // if ((tsAppLED_Blink.u8State) && (u32BlinkTmout < millis())) {
    //     u32BlinkTmout =  millis() + tsAppLED_Blink.u16Period;
    //     u8Toggle ^= 1;
    //     FastLED.clear();
    //     ledStrip[tsAppLED_Blink.u8Pos] = u8Toggle ? tsAppLED_Blink.Color : CRGB::Black;
    // }

    if (u32Timeout < u32RightNow) {
        u32Timeout = u32RightNow + LED_REFRESH;

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
    }

    if (u32ShiftedTimeout < u32NowShifted) {
        u32ShiftedTimeout = u32NowShifted + LED_REFRESH;
        FastLED.show();
    }
}

#endif // APP_FASTLED

/**
 * @file SubStrip.h
 * @brief Header file for the SubStrip class.
 * @author Nello
 * @date 2025-10-14
 */

#ifndef _SUBSTRIP_H
#define _SUBSTRIP_H

#include <FastLED.h>

 #define SUBSTRIP_STOP_PERIODIC (uint32_t)(-1)

class SubStrip {
public:
    typedef enum {
        NONE,
        GLITTER,
        RAINDROPS,
        FIRE,
        CHECKERED,
        NB_ANIMS
    } TeAnimation;

    SubStrip(uint8_t u8NbLeds);
    ~SubStrip();
    void vGetSubStrip(CRGB *leds, uint8_t u8NbLeds);
    void vManageAnimation(uint32_t u32Now); // to be called into loop()
    void vSetAnimation(TeAnimation eAnim);
    void vSetAnimation(TeAnimation eAnim, uint32_t u32Period, uint8_t u8Speed);
    void vSetAnimation(TeAnimation eAnim, uint32_t u32Period, uint8_t u8Speed, CRGB *pPalette);
    void vSetColorPalette(CRGB *ColorPalette);
    void vTriggerAnim(void);
    void vSetSpeed(uint8_t u8Speed);
    void vSetPeriod(uint32_t u32Period);
    void vClear(void);
    void vFillColor(CRGB color);
    bool bIsBlack(void);

private:
    /* Global object parameter */
    CRGB *_SubLeds;  // Pointer to the LED array
    CRGB *_ColorPalette; // Pointer to the color palette
    uint8_t _u8ColorNb; // Number of colors in the palette
    uint8_t _u8NbLeds; // Number of LEDs in the sub-strip
    uint8_t _u8Speed; // Animation speed
    uint32_t _u32Period; // Animation period
    uint32_t _u32Timeout; // Current time for animation timing

    /* Animation parameters */
    uint8_t _u8Index;
    uint8_t _u8DelayRate;
    CRGB *_pPixel;

    bool _bTrigger;
    TeAnimation _eCurrentAnimation = NONE;
    void vShiftFwd(CRGB *Color);
    void vInsertFwd(CRGB ColorFeed);
    void vShiftBwd(CRGB *Color);
    void vInsertBwd(CRGB ColorFeed);
    void vAnimateGlitter(void);
    void vAnimateRaindrops(void);
    void vAnimateFire(void);
    void vAnimateCheckered(void);
    void initCheckered(void);
};

#endif // _SUBSTRIP_H

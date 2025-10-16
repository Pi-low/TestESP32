/**
 * @file SubStrip.h
 * @brief Header file for the SubStrip class.
 * @author Nello
 * @date 2025-10-14
 */

#ifndef _SUBSTRIP_H
#define _SUBSTRIP_H

#include <FastLED.h>

class SubStrip {
public:
    typedef enum {
        NONE,
        GLITTER,
        RAINDROPS,
        FIRE,
        NB_ANIMS
    } TeAnimation;

    SubStrip(uint8_t u8NbLeds);
    ~SubStrip();
    void vGetSubStrip(CRGB *leds, uint8_t u8NbLeds);
    void vManageAnimation(uint32_t u32CurrentTick); // to be called into loop()
    void vSetAnimation(TeAnimation eAnim);
    void vSetSpeed(uint16_t u16Speed);
    void vSetPeriod(uint32_t u32Period);
    void vClear(void);
    void vFillColor(CRGB color);
    bool bIsBlack(void);

private:
    CRGB *_SubLeds;  // Pointer to the LED array
    CRGB *_ColorPalette; // Pointer to the color palette
    uint8_t _u8NbLeds; // Number of LEDs in the sub-strip
    uint16_t _u16Speed; // Animation speed
    uint32_t _u32Period; // Animation period
    uint32_t _u32CurrentTick;
    TeAnimation _eCurrentAnimation = NONE;
    void vShiftFwd(CRGB *Color);
    void vInsertFwd(CRGB ColorFeed);
    void vShiftBwd(CRGB *Color);
    void vInsertBwd(CRGB ColorFeed);
    void vAnimateGlitter(void);
    void vAnimateRaindrops(void);
    void vAnimateFire(void);
};

#endif // _SUBSTRIP_H

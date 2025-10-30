/**
 * @file SubStrip.h
 * @brief Header file for the SubStrip class.
 * @author Nello
 * @date 2025-10-14
 */

#ifndef _SUBSTRIP_H
#define _SUBSTRIP_H

#include <FastLED.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

class SubStrip {
public:
    typedef enum {
        RET_WARNING             = 1,
        RET_OK                  = 0,
        RET_GENERIC_ERROR       = -1,
        RET_BAD_PARAMETER       = RET_GENERIC_ERROR - 1,
        RET_INTERNAL_ERROR      = RET_GENERIC_ERROR - 2,
    } TeRetVal;

    typedef enum {
        NONE,
        GLITTER,
        RAINDROPS,
        FIRE,
        CHECKERED,
        NB_ANIMS
    } TeAnimation;

    typedef enum {
        FORWARD_INOUT,
        REVERSE_OUTIN
    } TeDirection;

    SubStrip(uint8_t u8NbLeds, CRGB *pLeds);
    ~SubStrip();
    TeRetVal eGetSubStrip(CRGB *leds, uint8_t u8NbLeds);
    void vManageAnimation(uint32_t u32Now); // to be called into loop()
    TeRetVal eSetAnimation(TeAnimation eAnim);
    TeRetVal eSetAnimation(TeAnimation eAnim, CRGB *pPalette);
    TeRetVal eSetAnimation(TeAnimation eAnim, CRGB *pPalette, uint32_t u32Period);
    TeRetVal eSetAnimation(TeAnimation eAnim, CRGB *pPalette, uint32_t u32Period, uint8_t u8Speed);
    TeRetVal eSetColorPalette(CRGB *ColorPalette);
    void vTriggerAnim(void);
    TeRetVal eSetSpeed(uint8_t u8Speed);
    TeRetVal eSetPeriod(uint32_t u32Period);
    TeRetVal eSetFadeRate(uint16_t u16FadeDelay);
    TeRetVal eSetDirection(TeDirection eDirection);
    TeRetVal eSetOffset(uint8_t u8Offset);
    void vClear(void);
    void vFillColor(CRGB color);
    bool bIsBlack(void);

private:
    /* Global object parameter */
    CRGB *_SubLeds;  // Pointer to the LED array
    CRGB *_ColorPalette; // Pointer to the color palette
    bool _bDynamic; //dynamic memory allocation of CRGB substrip
    uint8_t _u8ColorNb; // Number of colors in the palette
    uint8_t _u8NbLeds; // Number of LEDs in the sub-strip
    uint8_t _u8Speed; // Animation speed
    uint32_t _u32Period; // Animation period
    uint32_t _u32Timeout; // Current time for animation timing
    TeDirection _eDirection = FORWARD_INOUT;

    /* Animation parameters */
    uint8_t _u8Index;
    uint8_t _u8DelayRate;
    uint8_t _u8FadeRate;
    uint8_t _u8Offset;
    CRGB *_pPixel;

    bool _bTrigger;
    TeAnimation _eCurrentAnimation = NONE;
    void vShiftFwd(CRGB *Color);
    void vInsertFwd(CRGB ColorFeed);
    void vShiftBwd(CRGB *Color);
    void vInsertBwd(CRGB ColorFeed);
    uint8_t u8FadeTimeToRate(uint16_t u16FadeTime);
    void vAnimateGlitter(void);
    void vAnimateRaindrops(void);
    void vAnimateFire(void);
    void vAnimateCheckered(void);
    TeRetVal eInitCheckered(void);
};

#endif // _SUBSTRIP_H

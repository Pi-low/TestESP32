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

#define SUBSTRIP_STOP_PERIODIC     (uint32_t)(-1)
#define SUBSTRIP_SECURE_LOOOP      30
#define SUBSTRIP_FPS               50
#define _SUBSTRIP_PERIOD           (1000/SUBSTRIP_FPS)

typedef enum {
        SUBSTRP_WARNING             = 1,
        SUBSTRP_OK                  = 0,
        SUBSTRP_GENERIC_ERROR       = -1,
        SUBSTRP_BAD_PARAMETER       = SUBSTRP_GENERIC_ERROR - 1,
        SUBSTRP_INTERNAL_ERROR      = SUBSTRP_GENERIC_ERROR - 2,
} TeSubstrip_RetVal;

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

    typedef enum {
        FORWARD_INOUT,
        REVERSE_OUTIN
    } TeDirection;

    typedef struct {
        TeAnimation eAnimation;
        uint32_t u32Period;
        uint32_t u32TimOffset; //aka phase shift
        uint8_t u8Speed;
        TeDirection eDirection;
        CRGB* pColors;
    } TstConfig;

    SubStrip(uint8_t u8NbLeds, CRGB *pLeds);
    ~SubStrip();
    TeSubstrip_RetVal eGetSubStrip(CRGB *leds, uint8_t u8NbLeds);
    void vManageAnimation(uint32_t u32Now); // to be called into loop()
    TeSubstrip_RetVal eSetAnimation(TeAnimation eAnim);
    TeSubstrip_RetVal eSetAnimation(TeAnimation eAnim, CRGB *pPalette);
    TeSubstrip_RetVal eSetAnimation(TeAnimation eAnim, CRGB *pPalette, uint32_t u32Period);
    TeSubstrip_RetVal eSetAnimation(TeAnimation eAnim, CRGB *pPalette, uint32_t u32Period, uint8_t u8Speed);
    TeSubstrip_RetVal eSetColorPalette(CRGB *ColorPalette);
    void vTriggerAnim(void);
    TeSubstrip_RetVal eSetSpeed(uint8_t u8Speed);
    TeSubstrip_RetVal eSetPeriod(uint32_t u32Period);
    TeSubstrip_RetVal eSetFadeRate(uint16_t u16FadeDelay);
    TeSubstrip_RetVal eSetDirection(TeDirection eDirection);
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
    TeSubstrip_RetVal eInitCheckered(void);
};

#endif // _SUBSTRIP_H

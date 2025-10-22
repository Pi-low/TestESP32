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
 #define SUBSTRIP_SECURE_LOOOP  30

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
        uint8_t u8Speed,
        TeDirection eDirection;
        CRGB* pColors;
    } TstConfig;

    SubStrip(uint8_t u8NbLeds, CRGB *pLeds);
    ~SubStrip();
    void vGetSubStrip(CRGB *leds, uint8_t u8NbLeds);
    void vManageAnimation(uint32_t u32Now); // to be called into loop()
    void vSetAnimation(TeAnimation eAnim);
    void vSetAnimation(TeAnimation eAnim, CRGB *pPalette);
    void vSetAnimation(TeAnimation eAnim, CRGB *pPalette, uint32_t u32Period);
    void vSetAnimation(TeAnimation eAnim, CRGB *pPalette, uint32_t u32Period, uint8_t u8Speed);
    void vSetColorPalette(CRGB *ColorPalette);
    void vTriggerAnim(void);
    void vSetSpeed(uint8_t u8Speed);
    void vSetPeriod(uint32_t u32Period);
    void vSetDirection(TeDirection eDirection);
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
    TeDirection _eDirection;

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

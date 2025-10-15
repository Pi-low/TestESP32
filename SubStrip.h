/**
 * @file SubStrip.h
 * @brief Header file for the SubStrip class.
 * @author Nello
 * @date 2025-10-14
 */

#ifndef _SUBSTRIP_H
#define _SUBSTRIP_H

#include <FastLED.h>

typedef enum {
    SubStrip_NONE,
    SubStrip_GLITTER,
    SubStrip_RAINDROPS,
    SubStrip_FIRE
} TeAnimation;

class SubStrip {
public:
    SubStrip(uint8_t u8NbLeds);
    void GetSubStrip(CRGB *leds, uint8_t u8NbLeds);
    void ManageAnimation(void); // to be called into loop()
    void SetAnimation(TeAnimation eAnim, CRGB* ColorPalette, uint16_t *pu16Param);
    void Blackout(void);
    void fillColor(CRGB color);
    ~SubStrip();

private:
    CRGB *_SubLeds;  // Pointer to the LED array
    CRGB *_ColorPalette; // Pointer to the color palette
    uint8_t _u8NbLeds; // Number of LEDs in the sub-strip
    TeAnimation _eCurrentAnimation = SubStrip_NONE;
    void clear(void);
    void fadeToBlack(uint8_t u8FadeValue);
    void shiftFwd(CRGB *Color);
    void insertFwd(CRGB ColorFeed);
    void shiftBwd(CRGB *Color);
    void insertBwd(CRGB ColorFeed);;
    void AnimateGlitter(void);
    void AnimateRaindrops(void);
    void AnimateFire(void);
};

#endif // _SUBSTRIP_H

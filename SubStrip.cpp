/**
 * @file SubStrip.cpp
 * @brief Implementation of the SubStrip class.
 * @author Nello
 * @date 2025-10-14
 */

#include "SubStrip.h"

/**
 * @brief Constructor for the SubStrip class.
 * @param u8NbLeds Number of LEDs in the sub-strip.
 */
SubStrip::SubStrip(uint8_t u8NbLeds) : _u8NbLeds(u8NbLeds) {
    _SubLeds = new CRGB[_u8NbLeds];
    clear();
}

/**
 * @brief Clears the LED array by setting all LEDs to black.
 */
SubStrip::~SubStrip() {
    delete[] _SubLeds;
}

void SubStrip::GetSubStrip(CRGB *leds, uint8_t u8NbLeds) {
    if (u8NbLeds > _u8NbLeds) {
        // Handle error: number of LEDs does not match
        return;
    }
    memcpy(leds, _SubLeds, u8NbLeds * sizeof(CRGB));
}

/**
 * @brief Manage animations within the sub-strip.
 */
void SubStrip::ManageAnimation(void) {
    // Placeholder for managing animations
    switch (_eCurrentAnimation) {
        case ANIMATION_GLITTER:
            // Call glitter animation function
            break;
        case ANIMATION_RAINDROPS:
            // Call raindrops animation function
            break;
        case ANIMATION_FIRE:
            // Call fire animation function
            break;
        default:
            break;
    }
}

void SubStrip::SetGlitter(CRGB* ColorPalette, uint8_t u8FadeRate) {
    _ColorPalette = ColorPalette;
    _eCurrentAnimation = ANIMATION_GLITTER;
    // Additional setup for glitter animation
}

void SubStrip::SetRaindrops(CRGB SeedColor, uint8_t u8Speed) {
    _eCurrentAnimation = ANIMATION_RAINDROPS;
    // Additional setup for raindrops animation
}

void SubStrip::SetFire(void) {
    _eCurrentAnimation = ANIMATION_FIRE;
    // Additional setup for fire animation
}

void SubStrip::AnimateGlitter(void) {
    // Placeholder for glitter animation
}

void SubStrip::AnimateRaindrops(void) {
    // Placeholder for raindrops animation
}

void SubStrip::AnimateFire(void) {
    // Placeholder for fire animation
}

void SubStrip::clear(void) {
    memset(_SubLeds, 0, _u8NbLeds * sizeof(CRGB));
}

void SubStrip::fadeToBlack(uint8_t u8FadeValue) {
    CRGB* pLeds = _SubLeds;
    for (uint8_t i = 0; i < _u8NbLeds; i++) {
        pLeds->fadeToBlackBy(u8FadeValue);
        pLeds++;
    }
}
void SubStrip::shiftFwd(CRGB *Color) {
    CRGB last = (Color != NULL) ? *Color : _SubLeds[_u8NbLeds - 1];
    CRGB* pLeds = _SubLeds + _u8NbLeds - 1;

    for (uint8_t i = 0; i < _u8NbLeds; i++) {
        *pLeds = *(pLeds - 1);
        pLeds--;
    }
    _SubLeds[0] = last;
}

void SubStrip::insertFwd(CRGB ColorFeed) {
    shiftFwd(&ColorFeed);
}

void SubStrip::shiftBwd(CRGB *Color) {
    CRGB first = (Color != NULL) ? *Color : _SubLeds[0];
    CRGB* pLeds = _SubLeds;

    for (uint8_t i = 0; i < _u8NbLeds - 1; i++) {
        *pLeds = *(pLeds + 1);
        pLeds++;
    }
    _SubLeds[_u8NbLeds - 1] = first;
}

void SubStrip::insertBwd(CRGB ColorFeed) {
    shiftBwd(&ColorFeed);
}

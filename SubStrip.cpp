/**
 * @file SubStrip.cpp
 * @brief Implementation of the SubStrip class.
 * @author Nello
 * @date 2025-10-14
 */

#include "SubStrip.h"

/******************************************************************************/
/* Public methods                                                             */
/******************************************************************************/

/*******************************************************************************
 * @brief Constructor for the SubStrip class.
 * @param u8NbLeds Number of LEDs in the sub-strip.
 ******************************************************************************/
SubStrip::SubStrip(uint8_t u8NbLeds) {
    u8NbLeds = (u8NbLeds < 1) ? 1 : u8NbLeds; // Ensure at least one LED
    u8NbLeds = (u8NbLeds > 200) ? 200 : u8NbLeds; // Ensure at most 200 LEDs
    _u8NbLeds = u8NbLeds;
    _SubLeds = new CRGB[_u8NbLeds];
    _u32Period = 2000;
    _u16Speed = 33;
    _u32CurrentTick = 0;
    vClear();
}

/*******************************************************************************
 * @brief Clears the LED array by setting all LEDs to black.
 ******************************************************************************/
SubStrip::~SubStrip() {
    delete[] _SubLeds;
}

/*******************************************************************************
 * @brief Copies the sub-strip LED content to the provided LED array.
 * @param leds Pointer to the destination LED array.
 * @param u8NbLeds Number of LEDs to copy.
 ******************************************************************************/
void SubStrip::vGetSubStrip(CRGB *leds, uint8_t u8NbLeds) {
    if ((u8NbLeds > _u8NbLeds) || (leds == NULL)) {
        // Handle error: number of LEDs does not match
        return;
    }
    memcpy(leds, _SubLeds, u8NbLeds * sizeof(CRGB));
}

/*******************************************************************************
 * @brief Manage animations within the sub-strip.
 ******************************************************************************/
void SubStrip::vManageAnimation(uint32_t u32CurrentTick) {
    // Placeholder for managing animations
    _u32CurrentTick = u32CurrentTick;
    switch (_eCurrentAnimation) {
        case SubStrip::GLITTER:
            // Call glitter animation function
            vAnimateGlitter();
            break;
        case SubStrip::RAINDROPS:
            // Call raindrops animation function
            vAnimateRaindrops();
            break;
        case SubStrip::FIRE:
            // Call fire animation function
            vAnimateFire();
            break;
        default:
            break;
    }
}

/*******************************************************************************
 * @brief Set animation
 ******************************************************************************/
void SubStrip::vSetAnimation(TeAnimation eAnim) {
    if (eAnim >= SubStrip::NB_ANIMS) {
        return; // Invalid animation type
    }
    _eCurrentAnimation = eAnim;
}

/*******************************************************************************
 * @brief Set animation speed
 ******************************************************************************/
void SubStrip::vSetSpeed(uint16_t u16Speed) {
    _u16Speed = u16Speed;
}

/*******************************************************************************
 * @brief Set animation period
 ******************************************************************************/
void SubStrip::vSetPeriod(uint32_t u32Period) {
    _u32Period = u32Period;
}

/*******************************************************************************
 * @brief Clear the sub-strip by setting all LEDs to black.
 ******************************************************************************/
void SubStrip::vClear(void) {
    memset(_SubLeds, 0, _u8NbLeds * sizeof(CRGB));
}

/*******************************************************************************
 * @brief Fill the sub-strip with a specific color.
 * @param color The color to fill the sub-strip with.
 ******************************************************************************/
void SubStrip::vFillColor(CRGB color) {
    fill_solid(_SubLeds, _u8NbLeds, color);
}

/*******************************************************************************
 * @brief Check if all LEDs in the sub-strip are black.
 * @return true if all LEDs are black, false otherwise.
 ******************************************************************************/
bool SubStrip::bIsBlack(void) {
    CRGB *pPixel = _SubLeds;
    for (uint8_t i = 0; i < _u8NbLeds; i++) {
        if (*pPixel != CRGB::Black) {
            return false; // Not all LEDs are black
        }
        pPixel++;
    }
    return true; // All LEDs are black
}

/******************************************************************************/
/* Private methods                                                            */
/******************************************************************************/

void SubStrip::vShiftFwd(CRGB *Color) {
    CRGB last = (Color != NULL) ? *Color : _SubLeds[_u8NbLeds - 1];
    CRGB* pLeds = _SubLeds + _u8NbLeds - 1;

    for (uint8_t i = 0; i < _u8NbLeds; i++) {
        *pLeds = *(pLeds - 1);
        pLeds--;
    }
    _SubLeds[0] = last;
}

void SubStrip::vInsertFwd(CRGB ColorFeed) {
    vShiftFwd(&ColorFeed);
}

void SubStrip::vShiftBwd(CRGB *Color) {
    CRGB first = (Color != NULL) ? *Color : _SubLeds[0];
    CRGB* pLeds = _SubLeds;

    for (uint8_t i = 0; i < _u8NbLeds - 1; i++) {
        *pLeds = *(pLeds + 1);
        pLeds++;
    }
    _SubLeds[_u8NbLeds - 1] = first;
}

void SubStrip::vInsertBwd(CRGB ColorFeed) {
    vShiftBwd(&ColorFeed);
}

void SubStrip::vAnimateGlitter() {
    // Placeholder for glitter animation
}

void SubStrip::vAnimateRaindrops() {
    static uint8_t index = 0;
    static uint32_t u32Feed = 0;
    static uint32_t u32Timeout = 0;
    static CRGB *CurrentPixel = NULL;

    if (u32Timeout < _u32CurrentTick) {
        u32Timeout = _u32CurrentTick + _u16Speed;
        fadeToBlackBy(_SubLeds, _u8NbLeds, 90);
        if ((CurrentPixel != NULL) && (index < _u8NbLeds)) {
            *CurrentPixel = CRGB::White;
            index++;
            CurrentPixel++;
        }
    }

    if (u32Feed < _u32CurrentTick) {
        u32Feed = _u32CurrentTick + _u32Period;
        index = 0;
        CurrentPixel = _SubLeds;
    }
}

void SubStrip::vAnimateFire() {
    // Placeholder for fire animation
}

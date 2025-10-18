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
    _ColorPalette = NULL;
    _SubLeds = new CRGB[_u8NbLeds];
    _u32Period = 2000;
    _u32Timeout = 0;
    _bTrigger = false;
    _u8Speed = 1;

    /* Init animation parameters */
    _u8Index = 0;
    _u8DelayRate = 0;
    _pPixel = NULL;
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
void SubStrip::vManageAnimation(uint32_t u32Now) {
    switch (_eCurrentAnimation) {
        case SubStrip::GLITTER:
            // Call glitter animation function
            vAnimateGlitter();
            break;

        case SubStrip::RAINDROPS:
            // Call raindrops animation function
            if ((_u32Timeout < u32Now) && (_u32Period != SUBSTRIP_STOP_PERIODIC)) {
                _u32Timeout = u32Now + _u32Period;
                _bTrigger = true;
            }
            vAnimateRaindrops();
            break;

        case SubStrip::FIRE:
            // Call fire animation function
            vAnimateFire();
            break;

        case SubStrip::CHECKERED:
            // Call checkered animation function
            vAnimateCheckered();
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
    switch(eAnim) {
        case SubStrip::CHECKERED:
            initCheckered();
            break;
        default:
            break;
    }
    _eCurrentAnimation = eAnim;
}

void SubStrip::vSetAnimation(TeAnimation eAnim, uint32_t u32Period, uint8_t u8Speed) {
    vSetAnimation(eAnim);
    vSetPeriod(u32Period);
    vSetSpeed(u8Speed);
}

void SubStrip::vSetAnimation(TeAnimation eAnim, uint32_t u32Period, uint8_t u8Speed, CRGB *pPalette) {
    vSetAnimation(eAnim, u32Period, u8Speed);
    vSetColorPalette(pPalette);
}

/*******************************************************************************
 * @brief Set color palette
 ******************************************************************************/
void SubStrip::vSetColorPalette(CRGB *ColorPalette) {
    CRGB *pColor = ColorPalette;

    /* Protect from bad parameters */
    if (ColorPalette == NULL)
    { return; }
    else if (*ColorPalette == CRGB::Black)
    {  return; }

    /* Delete previous color palette */
    if (_ColorPalette) {
        delete[] _ColorPalette;
    }

    /* auto-detect nomber of colors */
    _u8ColorNb = 0;
    while (*pColor && (*pColor != CRGB::Black)) {
        _u8ColorNb++;
        pColor++;
    }

    /* Copy colors inside object */
    _ColorPalette = new CRGB[_u8ColorNb];
    memcpy(_ColorPalette, ColorPalette, _u8ColorNb * sizeof(CRGB));
}

/*******************************************************************************
 * @brief Trigger animation
 ******************************************************************************/
void SubStrip::vTriggerAnim(void)
{
    _bTrigger = true;
}

/*******************************************************************************
 * @brief Set animation speed, 1: fast, 255: slow
 * @details speed is expressed a multiple of animation callrate
 ******************************************************************************/
void SubStrip::vSetSpeed(uint8_t u8Speed) {
    _u8Speed = u8Speed ? u8Speed : 1;
}

/*******************************************************************************
 * @brief Set animation period
 ******************************************************************************/
void SubStrip::vSetPeriod(uint32_t u32Period) {
    _u32Period = u32Period ? u32Period : 100;
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
        if (*pPixel != CRGB::Black)
        { return false; }
        pPixel++;
    }
    return true; // All LEDs are black
}

/******************************************************************************/
/* Private methods                                                            */
/******************************************************************************/

/*******************************************************************************
 * @brief Shift leds forward (Din -> Dout)
 * @param Color pointer to color to feed, NULL will feed last color back
 ******************************************************************************/
void SubStrip::vShiftFwd(CRGB *Color) {
    CRGB last = (Color != NULL) ? *Color : _SubLeds[_u8NbLeds - 1];
    CRGB* pLeds = _SubLeds + _u8NbLeds - 1;

    for (uint8_t i = 0; i < _u8NbLeds; i++) {
        *pLeds = *(pLeds - 1);
        pLeds--;
    }
    _SubLeds[0] = last;
}

/*******************************************************************************
 * @brief Insert and shift leds forward (Din -> Dout)
 * @param ColorFeed color to insert inside the substrip
 ******************************************************************************/
void SubStrip::vInsertFwd(CRGB ColorFeed) {
    vShiftFwd(&ColorFeed);
}

/*******************************************************************************
 * @brief Shift leds backward (Dout -> Din)
 * @param Color pointer to color to feed, NULL will feed first color back
 ******************************************************************************/
void SubStrip::vShiftBwd(CRGB *Color) {
    CRGB first = (Color != NULL) ? *Color : _SubLeds[0];
    CRGB* pLeds = _SubLeds;

    for (uint8_t i = 0; i < _u8NbLeds - 1; i++) {
        *pLeds = *(pLeds + 1);
        pLeds++;
    }
    _SubLeds[_u8NbLeds - 1] = first;
}

/*******************************************************************************
 * @brief Insert and shift leds backward (Dout -> Din)
 * @param ColorFeed color to insert inside the substrip
 ******************************************************************************/
void SubStrip::vInsertBwd(CRGB ColorFeed) {
    vShiftBwd(&ColorFeed);
}

void SubStrip::vAnimateGlitter() {
    // Placeholder for glitter animation
}

/*******************************************************************************
 * @brief Manage raindrop animation
 ******************************************************************************/
void SubStrip::vAnimateRaindrops() {
    fadeToBlackBy(_SubLeds, _u8NbLeds, 90);

    if (_bTrigger && bIsBlack() && ((_u8Index >= _u8NbLeds) || !_u8Index)) {
        _bTrigger = false;
        _u8Index = 0;
        _pPixel = _SubLeds;
    }

    if ((_u8DelayRate % _u8Speed) == 0) {
        _u8DelayRate = 0;
        if ((_pPixel != NULL) && (_u8Index < _u8NbLeds)) {
            *_pPixel = CRGB::White;
            _u8Index++;
            _pPixel++;
        }
    }
    _u8DelayRate++;
}

void SubStrip::vAnimateFire() {
    // Placeholder for fire animation
}

/*******************************************************************************
 * @brief Manage chechered animation
 ******************************************************************************/
void SubStrip::vAnimateCheckered() {
    // Placeholder for checkered animation
    if ((_u8DelayRate % _u8Speed) == 0) {
        _u8DelayRate = 0;
        vShiftBwd(NULL);
    }
    _u8DelayRate++;
}

/*******************************************************************************
 * @brief Initialize checkered animation
 ******************************************************************************/
void SubStrip::initCheckered() {
    uint8_t u8Repeat = _u8NbLeds / _u8ColorNb;
    CRGB* pLed = _SubLeds;
    CRGB* pColor = _ColorPalette;

    for (uint8_t i = 0; i < _u8NbLeds; i++) {
        if ((i % u8Repeat == 0) && i) {
            pColor++;
            if ((pColor - _ColorPalette) >= _u8ColorNb) {
                pColor = _ColorPalette;
            }
        }
        *pLed = *pColor;
        pLed++;
    }
}

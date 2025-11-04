/**
 * @file SubStrip.cpp
 * @brief Implementation of the SubStrip class.
 * @author Nello
 * @date 2025-10-14
 */

#include "SubStrip.h"

#define SUBSTRIP_STOP_PERIODIC     (uint32_t)(-1)
#define SUBSTRIP_SECURE_LOOOP      30
#define _SUBSTRIP_PERIOD           (1000/SUBSTRIP_FPS)

#define _MNG_RETURN(x)  eRet = x

/******************************************************************************/
/* Public methods                                                             */
/******************************************************************************/

/*******************************************************************************
 * @brief Constructor for the SubStrip class.
 * @param u8NbLeds Number of LEDs in the sub-strip.
 ******************************************************************************/
SubStrip::SubStrip(uint8_t u8NbLeds, CRGB *pLeds) {
    u8NbLeds = (u8NbLeds < 1) ? 1 : u8NbLeds; // Ensure at least one LED
    u8NbLeds = (u8NbLeds > 200) ? 200 : u8NbLeds; // Ensure at most 200 LEDs
    _u8NbLeds = u8NbLeds;
    _ColorPalette = NULL;
    if (pLeds == NULL) { // dynamic allocation
        _SubLeds = new CRGB[_u8NbLeds];
        _bDynamic = true;
    }
    else {
        _SubLeds = pLeds; // use given pointer as strip reference
        _bDynamic = false;
    }
    
    _u32Period = 2000;
    _u32Timeout = 0;
    _bTrigger = false;
    _u8Speed = 1;
    _u8FadeRate = u8FadeTimeToRate(500);

    /* Init animation parameters */
    _u8Index = 0;
    _u8Bpm = 30;
    _u8DelayRate = 0;
    _pPixel = NULL;
    _u8Offset = 0;
    vClear();
}

/*******************************************************************************
 * @brief Clears the LED array by setting all LEDs to black.
 ******************************************************************************/
SubStrip::~SubStrip() {
    if (_bDynamic) {
        delete[] _SubLeds;
        _bDynamic = false;
    }
}

/*******************************************************************************
 * @brief Copies the sub-strip LED content to the provided LED array.
 * @param leds Pointer to the destination LED array.
 * @param u8NbLeds Number of LEDs to copy.
 ******************************************************************************/
SubStrip::TeRetVal SubStrip::eGetSubStrip(CRGB *leds, uint8_t u8NbLeds) {
    TeRetVal eRet = RET_OK;
    if ((u8NbLeds > _u8NbLeds) || (leds == NULL) || !_bDynamic) {
        _MNG_RETURN(RET_INTERNAL_ERROR);
    }
    else {
        memcpy(leds, _SubLeds, u8NbLeds * sizeof(CRGB));
    }
    return eRet;
}

/*******************************************************************************
 * @brief Manage animations within the sub-strip.
 ******************************************************************************/
void SubStrip::vManageAnimation(uint32_t u32Now) {
    switch (_eCurrentAnimation) {
        case GLITTER:
            // Call glitter animation function
            vAnimateGlitter();
            break;

        case RAINDROPS:
            // Call raindrops animation function
            if ((_u32Timeout < u32Now) && (_u32Period != SUBSTRIP_STOP_PERIODIC)) {
                _u32Timeout = u32Now + _u32Period;
                _bTrigger = true;
            }
            vAnimateRaindrops();
            break;

        case FIRE:
            // Call fire animation function
            vAnimateFire();
            break;

        case CHECKERED:
            // Call checkered animation function
            vAnimateCheckered();
            break;

        case WAVE:
            vAnimateWave();
            break;

        default:
            break;
    }
}

/*******************************************************************************
 * @brief Set animation
 ******************************************************************************/
SubStrip::SubStrip::TeRetVal SubStrip::eSetAnimation(TeAnimation eAnim) {
    TeRetVal eRet = RET_OK;
    if (eAnim >= SubStrip::NB_ANIMS) {
        _MNG_RETURN(RET_BAD_PARAMETER);
    }
    else {
        _u8DelayRate = 0;
        _u8Index = 0;

        switch(eAnim) {
            case SubStrip::CHECKERED:
                eRet = eInitCheckered();
                break;
            default:
                break;
        }
        _eCurrentAnimation = eAnim;
    }
    return eRet;
}

SubStrip::TeRetVal SubStrip::eSetAnimation(TeAnimation eAnim, CRGB *pPalette) {
    TeRetVal eRet = eSetColorPalette(pPalette);
    if (eRet >= RET_OK) {
        eRet = eSetAnimation(eAnim);
    }
    return eRet;
}

SubStrip::TeRetVal SubStrip::eSetAnimation(TeAnimation eAnim, CRGB *pPalette, uint32_t u32Period) {
    TeRetVal eRet = eSetColorPalette(pPalette);
    if (eRet >= RET_OK) {
        eRet = eSetAnimation(eAnim);
        eSetPeriod(u32Period);
    }
    return eRet;
}

SubStrip::TeRetVal SubStrip::eSetAnimation(TeAnimation eAnim, CRGB *pPalette, uint32_t u32Period, uint8_t u8Speed) {
    TeRetVal eRet = eSetColorPalette(pPalette);
    if (eRet >= RET_OK) {
        eRet = eSetAnimation(eAnim);
        eSetPeriod(u32Period);
        eSetSpeed(u8Speed);
    }
    return eRet;
}

/*******************************************************************************
 * @brief Set color palette
 ******************************************************************************/
SubStrip::TeRetVal SubStrip::eSetColorPalette(CRGB *ColorPalette) {
TeRetVal eRet = RET_OK;
    /* Protect from bad parameters */
    if (ColorPalette == NULL) {
        _MNG_RETURN(RET_BAD_PARAMETER);
    }
    else if (*ColorPalette == CRGB::Black) {
        _MNG_RETURN(RET_GENERIC_ERROR);
    }
    else {
        CRGB *pColor = ColorPalette;

        /* auto-detect nomber of colors */
        _u8ColorNb = 0;
        uint8_t u8SecureLoop = 0;
        while (*pColor && (*pColor != CRGB::Black) && (u8SecureLoop < SUBSTRIP_SECURE_LOOOP)) {
            _u8ColorNb++;
            pColor++;
            u8SecureLoop++;
        }

        if (_eCurrentAnimation == SubStrip::CHECKERED) {
            eRet = eInitCheckered();
        }

        _ColorPalette = ColorPalette;
    }
    return eRet;
}

/*******************************************************************************
 * @brief Trigger animation
 ******************************************************************************/
void SubStrip::vTriggerAnim(void) {
    _bTrigger = true;
}

/*******************************************************************************
 * @brief Set animation speed, 1: fast, 255: slow
 * @details speed is expressed a multiple of animation callrate
 * @param u8Speed [1-255] fast -> slow
 ******************************************************************************/
SubStrip::TeRetVal SubStrip::eSetSpeed(uint8_t u8Speed) {
    _u8Speed = u8Speed ? u8Speed : 1;
    return RET_OK;
}

/*******************************************************************************
 * @brief Set animation period
 * @param u32Period SUBSTRIP_STOP_PERIODIC will stop periodic triggering
 ******************************************************************************/
SubStrip::TeRetVal SubStrip::eSetPeriod(uint32_t u32Period) {
    _u32Period = u32Period ? u32Period : 100;
    return RET_OK;
}

/*******************************************************************************
 * @brief Set fading time for animation
 * @param u16FadeDelay Fading delay in milliseconds
 ******************************************************************************/
SubStrip::TeRetVal SubStrip::eSetFadeRate(uint16_t u16FadeDelay) {
    TeRetVal eRet = RET_OK;
    if (!u16FadeDelay) {
        _MNG_RETURN(RET_BAD_PARAMETER);
    }
    else {
        _u8FadeRate = u8FadeTimeToRate(u16FadeDelay);
#ifdef _TRACE_DBG
            _TRACE_DBG("[Substrip] vSetFadeRate -> set: %u\r\n", _u8FadeRate);
#endif
        if (!_u8FadeRate)
        { _u8FadeRate = 1; }
    }
    return eRet;
}

/*******************************************************************************
 * @brief Set animation period
 * @param eDirection
 ******************************************************************************/
SubStrip::TeRetVal SubStrip::eSetDirection(TeDirection eDirection) {
    TeRetVal eRet = RET_OK;
    if (eDirection > REVERSE_OUTIN) {
        _MNG_RETURN(RET_BAD_PARAMETER);
    }
    else {
        _eDirection = eDirection;
    }
    return eRet;
}

/*******************************************************************************
 * @brief Set the offset for the sub-strip
 * @param u8Offset The offset value
 ******************************************************************************/
SubStrip::TeRetVal SubStrip::eSetOffset(uint8_t u8Offset) {
    TeRetVal eRet = RET_OK;
    if (u8Offset > _u8NbLeds) {
        _MNG_RETURN(RET_BAD_PARAMETER);
    }
    else {
        _u8Offset = u8Offset;
    }
    return eRet;
}

/*******************************************************************************
 * @brief Set BPM rate
 * @param u8Bpm BPM rate
 ******************************************************************************/
SubStrip::TeRetVal SubStrip::eSetBpm(uint8_t u8Bpm) {
    TeRetVal eRet = RET_OK;
    if (!u8Bpm) {
        _MNG_RETURN(RET_BAD_PARAMETER);
    }
    else {
        _u8Bpm = u8Bpm;
    }
    return eRet;
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
    _u8Offset++;
    _u8Offset %= _u8NbLeds;
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
    _u8Offset++;
    _u8Offset %= _u8NbLeds;
}

/*******************************************************************************
 * @brief Insert and shift leds backward (Dout -> Din)
 * @param ColorFeed color to insert inside the substrip
 ******************************************************************************/
void SubStrip::vInsertBwd(CRGB ColorFeed) {
    vShiftBwd(&ColorFeed);
}

uint8_t SubStrip::u8FadeTimeToRate(uint16_t u16FadeTime) {
    return ((255*_SUBSTRIP_PERIOD)/u16FadeTime);
}

/*******************************************************************************
 * @brief Manage glitter animation
 ******************************************************************************/
void SubStrip::vAnimateGlitter() {
    // Placeholder for glitter animation
    fadeToBlackBy(_SubLeds, _u8NbLeds, _u8FadeRate);
    if ((_u8DelayRate % _u8Speed) == 0) {
        _u8DelayRate = 0;
        CRGB *pPixel = NULL;
        for (uint8_t i = 0; i < _u8ColorNb; i++) {
            pPixel = _SubLeds + (random8() % _u8NbLeds);
            if (*pPixel == CRGB::Black) {
                *pPixel = _ColorPalette[i];
            }
        }
    }
    _u8DelayRate++;
}

/*******************************************************************************
 * @brief Manage raindrop animation
 ******************************************************************************/
void SubStrip::vAnimateRaindrops() {
    if (_ColorPalette == NULL)
    { return; }
    
    if ((_u8DelayRate % _u8Speed) == 0)
    { fadeToBlackBy(_SubLeds, _u8NbLeds, _u8FadeRate); }

    if (_bTrigger && ((_u8Index >= _u8NbLeds) || !_u8Index)) {
        _bTrigger = false;
        _u8Index = 0;
        _pPixel = _SubLeds;
    }

    if ((_u8DelayRate % _u8Speed) == 0) {
        _u8DelayRate = 0;
        if ((_pPixel != NULL) && (_u8Index < _u8NbLeds)) {
            *_pPixel = *_ColorPalette;
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
        if (_eDirection == FORWARD_INOUT)
        { vShiftFwd(NULL); }
        else
        { vShiftBwd(NULL); }
    }
    _u8DelayRate++;
}

/*******************************************************************************
 * @brief Manage wave animation
 ******************************************************************************/
void SubStrip::vAnimateWave(void) {
    if (_ColorPalette && (_u8ColorNb >= 2)) {
        uint8_t u8Pos = beatsin8(_u8Bpm, 0, _u8NbLeds-6, 0, _u8Offset);
        vClear();
        fill_solid(_SubLeds, u8Pos, _ColorPalette[0]);
        fill_solid(_SubLeds + u8Pos, _u8NbLeds - u8Pos, _ColorPalette[1]);
        fill_gradient_RGB(_SubLeds + u8Pos, 6, _ColorPalette[0], _ColorPalette[1]);
    }
}

/*******************************************************************************
 * @brief Initialize checkered animation
 ******************************************************************************/
SubStrip::TeRetVal SubStrip::eInitCheckered() {
    TeRetVal eRet = RET_OK;
    if ((!_u8ColorNb) || (_ColorPalette == NULL)){
        _MNG_RETURN(RET_INTERNAL_ERROR);
    }
    else {
        uint8_t u8Repeat = _u8NbLeds / _u8ColorNb;
        CRGB* pLed = _SubLeds;
        CRGB* pColor = _ColorPalette;

        for (uint8_t i = 0; i < _u8NbLeds; i++) {
            if (((i + _u8Offset) % u8Repeat == 0) && i) {
                pColor++;
                if ((pColor - _ColorPalette) >= _u8ColorNb) {
                    pColor = _ColorPalette;
                }
            }
            *pLed = *pColor;
            pLed++;
        }
    }
    return eRet;
}

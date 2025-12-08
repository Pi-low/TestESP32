/**
 * @brief WS2812 management
 * @file Lights.c
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */

#include "App_Leds.h"
#include "App_PrintUtils.h"
#include <list>

#if defined(APP_FASTLED) && APP_FASTLED

/*******************************************************************************
 *  CONFIGURATION
 ******************************************************************************/
#define LED_DATA_PIN        4
#define LED_CHIPSET         WS2812
#define LED_PIXEL_ORDER     GRB
#define LED_BRIGHTNESS      127
#define LED_CHANGE_DELAY    5000
#define LED_STATIC_PALETTE_NB  6

#if APP_TASKS
// APP_LEDS Task
#define LED_TASK            "APP_LEDS"
#define LED_TASK_HEAP       (configMINIMAL_STACK_SIZE*2)
#define LED_TASK_PARAM      NULL
#define LED_TASK_PRIO       2
#define LED_TASK_HANDLE     NULL

// APP_ANIM Task
#define ANIM_TASK           "APP_ANIM"
#define ANIM_TASK_HEAP      (configMINIMAL_STACK_SIZE*2)
#define ANIM_TASK_PARAM     NULL
#define ANIM_TASK_PRIO      2
#define ANIM_TASK_HANDLE    NULL
#endif

#define _LED_TIMEOUT        (1000/SUBSTRIP_FPS) //ms
#define _LED_NB             (LED_SUBSTRIP_LEN * LED_SUBSTRIP_NB)
#define _LED_SUB_OFFSET(x)  (x * LED_SUBSTRIP_LEN)
#define _LOOP_CNT_MS(x)     (x/_LED_TIMEOUT)

/*******************************************************************************
 *  TYPES, ENUM, DEFINITIONS 
 ******************************************************************************/
typedef struct {
    uint16_t u16NbLeds;
    uint8_t u8NbStrips;
    uint8_t* pu8Strips;
    CRGB* pLedStrip;
    SubStrip *SubStrips;
} TstStripCfg;

typedef struct {
    SubStrip::TeAnimation eAnimation;
    uint32_t u32Period;
    uint8_t u8Offset;
    uint8_t u8Speed;
    uint16_t u16MsFade;
    SubStrip::TeDirection eDirection;
    CRGB* pPalette;
} TstConfig;

/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
static TstStripCfg stAppLED_Config  = {0, 0, nullptr, nullptr, nullptr};
static CRGB pMyColorPalette1[3] = {CRGB::White, CRGB::Red, CRGB::Black};
static CRGB tCustomPalettes[LED_SUBSTRIP_NB][LED_STATIC_PALETTE_NB + 1] = {{{CRGB::Black}}};

static CRGB *ledStrip;
static SubStrip *SubStrips;


static const TstConfig AnimationConfig[LED_SUBSTRIP_NB] = {
//   Animation              Period   Offset  Speed   MsFade  Direction                   Palette
    {SubStrip::RAINDROPS,   2000,    0,      2,      100,    SubStrip::FORWARD_INOUT,    pMyColorPalette1},
    {SubStrip::RAINDROPS,   2000,    0,      2,      100,    SubStrip::FORWARD_INOUT,    pMyColorPalette1},
    {SubStrip::RAINDROPS,   2000,    0,      2,      100,    SubStrip::FORWARD_INOUT,    pMyColorPalette1},
    {SubStrip::RAINDROPS,   2000,    0,      2,      100,    SubStrip::FORWARD_INOUT,    pMyColorPalette1},
    {SubStrip::RAINDROPS,   2000,    0,      2,      100,    SubStrip::FORWARD_INOUT,    pMyColorPalette1},
};

static bool bAppLed_displayOn = false;

const char *tpcAppLED_Animations[SubStrip::NB_ANIMS] = {
    "none",
    "glitter",
    "raindrops",
    "checkered",
    "wave"
};

#if APP_TASKS
SemaphoreHandle_t xLedStripSema;
void vAppLedsTask(void *pvParam);
void vAppLedsAnimTask(void *pvParam);
#endif

/*******************************************************************************
 * @brief Initialize ledstrip
 * 
 ******************************************************************************/
void AppLED_init(void) {
    bool bStartTasking = false;
    JsonArray jStrips;
    bAppCfg_LockJson();
    jStrips = jAppCfg_Config["DEVICE_SUBSTRIPS"].as<JsonArray>();
    stAppLED_Config.u8NbStrips = jStrips.size();
    if (stAppLED_Config.u8NbStrips)
    {
        char tcPrint[128];
        snprintf(tcPrint, 128, "[AppLED_init] Loading %u strips: ", stAppLED_Config.u8NbStrips);
        stAppLED_Config.pu8Strips = (uint8_t*)pvPortMalloc(stAppLED_Config.u8NbStrips * sizeof(uint8_t));
        if (stAppLED_Config.pu8Strips)
        {
            uint8_t *ptr = stAppLED_Config.pu8Strips;
            for(uint8_t u8Val : jStrips)
            {
                *ptr = u8Val;
                stAppLED_Config.u16NbLeds += u8Val;
                snprintf(tcPrint + strlen(tcPrint), 128 - strlen(tcPrint), "%u ", u8Val);
                ptr++;
            }
            snprintf(tcPrint + strlen(tcPrint), 128 - strlen(tcPrint), "\r\nTotal ledstrip: %u\r\n", stAppLED_Config.u16NbLeds);
        }
        else 
        { snprintf(tcPrint + strlen(tcPrint), 128 - strlen(tcPrint), "[AppLED_init] Malloc error !\r\n"); }
        APP_TRACE(tcPrint);
    }
    else
    {
        APP_TRACE("[AppLED_init] No strip found into config file!\r\n");
    }
    bAppCfg_UnlockJson();

    if (stAppLED_Config.u16NbLeds && stAppLED_Config.u8NbStrips && stAppLED_Config.pu8Strips)
    {
        stAppLED_Config.pLedStrip = (CRGB*)pvPortMalloc(stAppLED_Config.u16NbLeds * sizeof(CRGB)); // Dynamic allocation
        stAppLED_Config.SubStrips = (SubStrip*)pvPortMalloc(stAppLED_Config.u8NbStrips * sizeof(SubStrip)); // Dynamic allocation
        if ((stAppLED_Config.pLedStrip != nullptr) && (stAppLED_Config.SubStrips != nullptr))
        {
            CRGB *ptrCRGB = stAppLED_Config.pLedStrip;
            for (uint8_t u8cnt = 0; u8cnt < stAppLED_Config.u8NbStrips; u8cnt++)
            {
                stAppLED_Config.SubStrips[u8cnt] = SubStrip(stAppLED_Config.pu8Strips[u8cnt], ptrCRGB); // create substrip assembly by reference
                ptrCRGB += stAppLED_Config.pu8Strips[u8cnt];
            }
            ledStrip = stAppLED_Config.pLedStrip;
            SubStrips = stAppLED_Config.SubStrips;
            FastLED.addLeds<LED_CHIPSET, LED_DATA_PIN, LED_PIXEL_ORDER>(ledStrip, stAppLED_Config.u16NbLeds);
            FastLED.setBrightness(LED_BRIGHTNESS);
            FastLED.setCorrection(TypicalLEDStrip);
            FastLED.clear();
            FastLED.show();
            TstConfig *pstConfig = (TstConfig *)AnimationConfig;
            SubStrip *pObj = SubStrips;
            char tcDbgString[PRINT_UTILS_MAX_BUF] = {0};
            for (uint8_t i = 0; i < stAppLED_Config.u8NbStrips; i++)
            {
                pObj->eSetOffset(pstConfig->u8Offset);
                pObj->eSetDirection(pstConfig->eDirection);
                pObj->eSetFadeRate(pstConfig->u16MsFade);
                if (pObj->eSetAnimation(pstConfig->eAnimation, pstConfig->pPalette, pstConfig->u32Period, pstConfig->u8Speed) < SubStrip::RET_OK)
                {
                    snprintf(tcDbgString, PRINT_UTILS_MAX_BUF, "[AppLED_init] SubStrip %u set animation failed\r\n", i);
                    APP_TRACE(tcDbgString);
                }
                // pstConfig++;
                pObj++;
            }
            bStartTasking = true;
        }
        else
        {
            APP_TRACE("[AppLED_init] ledstrip: malloc error!\r\n");
        }
    }

#if APP_TASKS
    if (bStartTasking)
    {
        xLedStripSema = xSemaphoreCreateBinary();
        if (xLedStripSema == NULL)
        {
            APP_TRACE("[AppLED_init] xLedStripSema create failed\r\n");
        }
        else
        {
            xSemaphoreGive(xLedStripSema);
            xTaskCreate(vAppLedsTask, LED_TASK, LED_TASK_HEAP, LED_TASK_PARAM, LED_TASK_PRIO, LED_TASK_HANDLE);
            APP_TRACE("[AppLED_init] Run task!\r\n");
        }
    }
#endif
}

#if APP_TASKS
/*******************************************************************************
 * @brief AppLeds main task
 * 
 ******************************************************************************/
void vAppLedsTask(void *pvParam) {
    char tcDbgString[PRINT_UTILS_MAX_BUF] = {0};
    uint8_t u8Toggle = 0;
    uint8_t u8SubIndex = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t u32Now;
    // uint16_t u16Cnt = 0;
    SubStrip *pObj;
    while (1) {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(_LED_TIMEOUT));
        if (LOCK_LEDS()) {
            // protected ressource >>>
            u32Now = millis();
            pObj = SubStrips;
            for (u8SubIndex = 0; u8SubIndex < stAppLED_Config.u8NbStrips; u8SubIndex++) {
                pObj->vManageAnimation(u32Now);
                pObj++;
            }
            if (!bAppLed_displayOn)
            { FastLED.clear(); }
            FastLED.show();
            // <<< end of protected ressource
            UNLOCK_LEDS();
        }
    }
}

/*******************************************************************************
 * @brief AppLeds animation change task
 * 
 ******************************************************************************/
void vAppLedsAnimTask(void *pvParam) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    SubStrip *pObj;
    char tcDbgString[PRINT_UTILS_MAX_BUF] = {0};
    while (1) {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(LED_CHANGE_DELAY));

        if(LOCK_LEDS()) {
            // protected ressource >>>
            snprintf(tcDbgString, PRINT_UTILS_MAX_BUF, "[APP_ANIM] [%u] Animation event\r\n", millis());
            APP_TRACE(tcDbgString);
            // <<< end of protected ressource
            UNLOCK_LEDS();
        }
    }
}
#else
/*******************************************************************************
 * @brief Self-timed ledstrip refresh
 * 
 ******************************************************************************/
void AppLED_showLoop(void) {
    SubStrip *pObj = SubStrips;
    static uint32_t u32Timeout = 0;
    uint32_t u32RightNow = millis();
    CRGB* Colors;

    if (u32Timeout < u32RightNow) {
        u32Timeout = u32RightNow + _LED_TIMEOUT;
        for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
            pObj->vManageAnimation(u32RightNow);
            pObj++;
        }
        FastLED.show();
    }
}
#endif

eApp_RetVal eAppLed_blackout(void) {
    bAppLed_displayOn = false;
    return eRet_Ok;
}

eApp_RetVal eAppLed_resume(void) {
    bAppLed_displayOn = true;
    return eRet_Ok;
}

eApp_RetVal eAppLed_SetBrightness(uint8_t u8Value) {
    FastLED.setBrightness(u8Value);
    return eRet_Ok;
}

eApp_RetVal eAppLed_SetAnimation(SubStrip::TeAnimation eAnimation, uint8_t u8Index) {
    eApp_RetVal eRet = eRet_Ok;
    if (eAnimation >= SubStrip::NB_ANIMS) {
        eRet = eRet_BadParameter;
    }
    else if (LOCK_LEDS()) {
        SubStrip *pObj = NULL;
        if (u8Index != _LED_ALLSTRIPS) {
            pObj = &SubStrips[u8Index];
            eRet = (pObj->eSetAnimation(eAnimation) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
        }
        else {
            pObj = SubStrips;
            for (uint8_t i = 0; (i < stAppLED_Config.u8NbStrips) && (eRet >= eRet_Ok); i++) {
                eRet = (pObj->eSetAnimation(eAnimation) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
                pObj++;
            }
        }
        UNLOCK_LEDS();
    }
    return eRet;
}

eApp_RetVal eAppLed_SetSpeed(uint8_t u8Speed, uint8_t u8Index) {
    eApp_RetVal eRet = eRet_Ok;
    if ((u8Index >= stAppLED_Config.u8NbStrips) && (u8Index != _LED_ALLSTRIPS)) {
        eRet = eRet_BadParameter;
    }
    else if (LOCK_LEDS()) {
        SubStrip *pObj = NULL;
        if (u8Index != _LED_ALLSTRIPS) {
            pObj = &SubStrips[u8Index];
            eRet = (pObj->eSetSpeed(u8Speed) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
        }
        else {
            pObj = SubStrips;
            for (uint8_t i = 0; (i < stAppLED_Config.u8NbStrips) && (eRet >= eRet_Ok); i++) {
                eRet = (pObj->eSetSpeed(u8Speed) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
                pObj++;
            }
        }
        UNLOCK_LEDS();
    }
    return eRet;
}

eApp_RetVal eAppLed_SetPeriod(uint32_t u32Period, uint8_t u8Index) {
    eApp_RetVal eRet = eRet_Ok;
    if ((u8Index >= stAppLED_Config.u8NbStrips) && (u8Index != _LED_ALLSTRIPS)) {
        eRet = eRet_BadParameter;
    }
    else if (LOCK_LEDS()) {
        SubStrip *pObj = NULL;
        if (u8Index != _LED_ALLSTRIPS) {
            pObj = &SubStrips[u8Index];
            eRet = (pObj->eSetPeriod(u32Period) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
        }
        else {
            pObj = SubStrips;
            for (uint8_t i = 0; (i < stAppLED_Config.u8NbStrips) && (eRet >= eRet_Ok); i++) {
                eRet = (pObj->eSetPeriod(u32Period) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
                pObj++;
            }
        }
        UNLOCK_LEDS();
    }
    return eRet;
}

eApp_RetVal eAppLed_SetFade(uint16_t u16FadeMs, uint8_t u8Index) {
    eApp_RetVal eRet = eRet_Ok;
    if ((u8Index >= stAppLED_Config.u8NbStrips) && (u8Index != _LED_ALLSTRIPS)) {
        eRet = eRet_BadParameter;
    }
    else if (LOCK_LEDS()) {
        SubStrip *pObj = NULL;
        if (u8Index != _LED_ALLSTRIPS) {
            pObj = &SubStrips[u8Index];
            eRet = (pObj->eSetFadeRate(u16FadeMs) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
        }
        else {
            pObj = SubStrips;
            for (uint8_t i = 0; (i < stAppLED_Config.u8NbStrips) && (eRet >= eRet_Ok); i++) {
                eRet = (pObj->eSetFadeRate(u16FadeMs) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
                pObj++;
            }
        }
        UNLOCK_LEDS();
    }
    return eRet;
}

eApp_RetVal eAppLed_SetDirection(SubStrip::TeDirection eDirection, uint8_t u8Index) {
    eApp_RetVal eRet = eRet_Ok;
    if ((u8Index >= stAppLED_Config.u8NbStrips) && (u8Index != _LED_ALLSTRIPS)) {
        eRet = eRet_BadParameter;
    }
    else if (LOCK_LEDS()) {
        SubStrip *pObj = NULL;
        if (u8Index != _LED_ALLSTRIPS) {
            pObj = &SubStrips[u8Index];
            eRet = (pObj->eSetDirection(eDirection) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
        }
        else {
            pObj = SubStrips;
            for (uint8_t i = 0; (i < stAppLED_Config.u8NbStrips) && (eRet >= eRet_Ok); i++) {
                eRet = (pObj->eSetDirection(eDirection) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
                pObj++;
            }
        }
        UNLOCK_LEDS();
    }
    return eRet;
}

eApp_RetVal eAppLed_SetOffset(uint8_t u8Offset, uint8_t u8Index) {
    eApp_RetVal eRet = eRet_Ok;
    if ((u8Index >= stAppLED_Config.u8NbStrips) && (u8Index != _LED_ALLSTRIPS)) {
        eRet = eRet_BadParameter;
    }
    else if (LOCK_LEDS()) {
        SubStrip *pObj = NULL;
        if (u8Index != _LED_ALLSTRIPS) {
            pObj = &SubStrips[u8Index];
            eRet = (pObj->eSetOffset(u8Offset) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
        }
        else {
            pObj = SubStrips;
            for (uint8_t i = 0; (i < stAppLED_Config.u8NbStrips) && (eRet >= eRet_Ok); i++) {
                eRet = (pObj->eSetOffset(u8Offset) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
                pObj++;
            }
        }
        UNLOCK_LEDS();
    }
    return eRet;
}

eApp_RetVal eAppLed_SetBpm(uint8_t u8Bpm, uint8_t u8Index) {
    eApp_RetVal eRet = eRet_Ok;
    if ((u8Index >= stAppLED_Config.u8NbStrips) && (u8Index != _LED_ALLSTRIPS)) {
        eRet = eRet_BadParameter;
    }
    else if (LOCK_LEDS()) {
        SubStrip *pObj = NULL;
        if (u8Index != _LED_ALLSTRIPS) {
            pObj = &SubStrips[u8Index];
            eRet = (pObj->eSetBpm(u8Bpm) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
        }
        else {
            pObj = SubStrips;
            for (uint8_t i = 0; (i < stAppLED_Config.u8NbStrips) && (eRet >= eRet_Ok); i++) {
                eRet = (pObj->eSetBpm(u8Bpm) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
                pObj++;
            }
        }
        UNLOCK_LEDS();
    }
    return eRet;
}

eApp_RetVal eAppLed_SetPalette(uint8_t u8PaletteIndex, uint8_t u8SubStripIndex) {
    eApp_RetVal eRet = eRet_Ok;
    if (((u8SubStripIndex >= stAppLED_Config.u8NbStrips) && (u8SubStripIndex != _LED_ALLSTRIPS)) || (u8PaletteIndex >= stAppLED_Config.u8NbStrips))
    { eRet = eRet_BadParameter; }
    else {
        CRGB *pPalette = &tCustomPalettes[u8PaletteIndex][0];
        SubStrip *pObj = NULL;

        if (LOCK_LEDS()) {
            if (u8SubStripIndex != _LED_ALLSTRIPS) {
                pObj = &SubStrips[u8SubStripIndex];
                eRet = (pObj->eSetColorPalette(pPalette) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
            }
            else {
                pObj = SubStrips;
                for (uint8_t i = 0; (i < stAppLED_Config.u8NbStrips) && (eRet >= eRet_Ok); i++) {
                    eRet = (pObj->eSetColorPalette(pPalette) < SubStrip::RET_OK) ? eRet_InternalError : eRet_Ok;
                    pObj++;
                }
            }
            UNLOCK_LEDS();
        }
    }
    return eRet;
}

eApp_RetVal eAppLed_LoadColorAt(CRGB xColor, uint8_t u8PaletteIndex, uint8_t u8Index) {
    eApp_RetVal eRet = eRet_Ok;
    if ((u8Index >= LED_STATIC_PALETTE_NB) || (u8PaletteIndex >= stAppLED_Config.u8NbStrips) || (xColor == CRGB::Black))
    { eRet = eRet_BadParameter; }
    else {
        if (LOCK_LEDS()) {
            tCustomPalettes[u8PaletteIndex][u8Index] = xColor;
            UNLOCK_LEDS();
        }
    }
    return eRet;
}

eApp_RetVal eAppLed_LoadColors(CRGB *xColor, uint8_t u8NbColors, uint8_t u8PaletteIndex) {
    eApp_RetVal eRet = eRet_Ok;
    if ((u8NbColors > LED_STATIC_PALETTE_NB) || (u8NbColors == 0) || (u8PaletteIndex >= stAppLED_Config.u8NbStrips))
    { eRet = eRet_BadParameter; }
    else {
        if (LOCK_LEDS()) {
            memset(tCustomPalettes[u8PaletteIndex], 0, sizeof(CRGB) * (LED_STATIC_PALETTE_NB + 1));
            memcpy(tCustomPalettes[u8PaletteIndex], xColor, sizeof(CRGB) * u8NbColors);
            UNLOCK_LEDS();
        }
    }
    return eRet;
}

#endif // APP_FASTLED

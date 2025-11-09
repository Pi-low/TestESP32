/**
 * @brief WS2812 management
 * @file Lights.c
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */

#include "App_Leds.h"
#include "App_PrintUtils.h"

#if defined(APP_FASTLED) && APP_FASTLED

#include "SubStrip.h"

/*******************************************************************************
 *  CONFIGURATION
 ******************************************************************************/
#define LED_DATA_PIN        4
#define LED_CHIPSET         WS2812
#define LED_PIXEL_ORDER     GRB
#define LED_BRIGHTNESS      127
#define LED_CHANGE_DELAY    5000

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
static CRGB pMyColorPalette1[3] = {CRGB::White, CRGB::Red, CRGB::Black};
static CRGB pMyColorPalette2[3] = {CRGB::Orange, CRGB::Fuchsia, CRGB::Black};

static CRGB ledStrip[_LED_NB];
static SubStrip SubStrips[LED_SUBSTRIP_NB] = {
    SubStrip(LED_SUBSTRIP_LEN, ledStrip + _LED_SUB_OFFSET(0)),
    SubStrip(LED_SUBSTRIP_LEN, ledStrip + _LED_SUB_OFFSET(1)),
    SubStrip(LED_SUBSTRIP_LEN, ledStrip + _LED_SUB_OFFSET(2)),
    SubStrip(LED_SUBSTRIP_LEN, ledStrip + _LED_SUB_OFFSET(3)),
    SubStrip(LED_SUBSTRIP_LEN, ledStrip + _LED_SUB_OFFSET(4)),
};

static const TstConfig AnimationConfig[LED_SUBSTRIP_NB] = {
//   Animation            Period   Offset  Speed   MsFade  Direction                   Palette
    {SubStrip::RAINDROPS, 800,     0,      2,      75,     SubStrip::FORWARD_INOUT,    pMyColorPalette2},
    {SubStrip::RAINDROPS, 800,     0,      2,      75,     SubStrip::FORWARD_INOUT,    pMyColorPalette2},
    {SubStrip::RAINDROPS, 800,     0,      2,      75,     SubStrip::FORWARD_INOUT,    pMyColorPalette2},
    {SubStrip::RAINDROPS, 800,     0,      2,      75,     SubStrip::FORWARD_INOUT,    pMyColorPalette2},
    {SubStrip::RAINDROPS, 800,     0,      2,      75,     SubStrip::FORWARD_INOUT,    pMyColorPalette2},
};

static bool bAppLed_displayOn = false;

const char *tpcAppLED_Animations[SubStrip::NB_ANIMS] = {
    "none",
    "glitter",
    "raindrops",
    "fire",
    "checkered",
    "wave"
}

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
    FastLED.addLeds<LED_CHIPSET, LED_DATA_PIN, LED_PIXEL_ORDER>(ledStrip, _LED_NB);
    FastLED.setBrightness(LED_BRIGHTNESS);
    FastLED.setCorrection(TypicalLEDStrip);
    FastLED.clear();
    FastLED.show();
    TstConfig *pstConfig = (TstConfig*)AnimationConfig;
    SubStrip *pObj = SubStrips;
    char tcDbgString[PRINT_UTILS_MAX_BUF] = {0};
    for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
        pObj->eSetOffset(pstConfig->u8Offset);
        pObj->eSetDirection(pstConfig->eDirection);
        pObj->eSetFadeRate(pstConfig->u16MsFade);
        if (pObj->eSetAnimation(pstConfig->eAnimation, pstConfig->pPalette, pstConfig->u32Period, pstConfig->u8Speed) < SubStrip::RET_OK) {
            snprintf(tcDbgString, PRINT_UTILS_MAX_BUF, "[AppLED_init] SubStrip %u set animation failed\r\n", i);
            APP_TRACE(tcDbgString);
        }
        pstConfig++;
        pObj++;
    }

#if APP_TASKS
    xLedStripSema = xSemaphoreCreateBinary();
    if (xLedStripSema == NULL)
    { APP_TRACE("[AppLED_init] xLedStripSema create failed\r\n"); }
    else {
        xSemaphoreGive(xLedStripSema);
        xTaskCreate(vAppLedsTask, LED_TASK, LED_TASK_HEAP, LED_TASK_PARAM, LED_TASK_PRIO, LED_TASK_HANDLE);
        // xTaskCreate(vAppLedsAnimTask, ANIM_TASK, ANIM_TASK_HEAP, ANIM_TASK_PARAM, ANIM_TASK_PRIO, ANIM_TASK_HANDLE);
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
            for (u8SubIndex = 0; u8SubIndex < LED_SUBSTRIP_NB; u8SubIndex++) {
                pObj->vManageAnimation(u32Now);
                pObj++;
            }
            if (!bAppLed_displayOn)
            { FastLED.clear(); }
            FastLED.show();
            // <<< end of protected ressource
            UNLOCK_LEDS();
        }
        
        // if ((u16Cnt % _LOOP_CNT_MS(1000)) == 0) {
        //     u16Cnt = 0;
        //     snprintf(tcDbgString, PRINT_UTILS_MAX_BUF, "[APP_LEDS] [%u] Task alive\r\n", millis(), xLedStripSema);
        //     APP_TRACE(tcDbgString);
        // }
        // u16Cnt++;
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

bool bAppLed_blackout(void) {
    bAppLed_displayOn = false;
    return true;
}

bool bAppLed_resume(void) {
    bAppLed_displayOn = true;
    return true;
}

bool bAppLed_SetBrightness(uint8_t u8Value) {
    FastLED.setBrightness(u8Value);
    return true;
}

bool bAppLed_SetSpeed(uint8_t u8Speed) {
    bool bRet = false;
    if (LOCK_LEDS()) {
        SubStrip *pObj = SubStrips;
        for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
            pObj->eSetSpeed(u8Speed);
            pObj++;
        }
        bRet = true;
        UNLOCK_LEDS();
    }
    return bRet;
}

bool bAppLed_SetPeriod(uint32_t u32Period) {
    bool bRet = false;
    if (LOCK_LEDS()) {
        SubStrip *pObj = SubStrips;
        for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
            pObj->eSetPeriod(u32Period);
            pObj++;
        }
        bRet = true;
        UNLOCK_LEDS();
    }
    return bRet;
}

bool bAppLed_SetFade(uint16_t u16FadeMs) {
    bool bRet = false;
    if (LOCK_LEDS()) {
        SubStrip *pObj = SubStrips;
        for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
            pObj->eSetFadeRate(u16FadeMs);
            pObj++;
        }
        bRet = true;
        UNLOCK_LEDS();
    }
    return bRet;
}

#endif // APP_FASTLED

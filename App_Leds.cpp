/**
 * @brief WS2812 management
 * @file Lights.c
 * @version 0.1
 * @date 2025-10-13
 * @author Nello
 */

#include "App_Leds.h"

#if defined(APP_FASTLED) && APP_FASTLED

#include "SubStrip.h"

static CRGB pMyColorPalette1[3] = {CRGB::White, CRGB::Red, CRGB::Black};
static CRGB pMyColorPalette2[3] = {CRGB::Orange, CRGB::Fuchsia, CRGB::Black};

typedef struct {
    SubStrip::TeAnimation eAnimation;
    uint32_t u32Period;
    uint8_t u8Offset;
    uint8_t u8Speed;
    SubStrip::TeDirection eDirection;
    CRGB* pPalette;
} TstConfig;

static CRGB ledStrip[_LED_NB];
static SubStrip SubStrips[LED_SUBSTRIP_NB] = {
    SubStrip(20, ledStrip + _LED_SUB_OFFSET(0)),
    SubStrip(20, ledStrip + _LED_SUB_OFFSET(1)),
    SubStrip(20, ledStrip + _LED_SUB_OFFSET(2)),
    SubStrip(20, ledStrip + _LED_SUB_OFFSET(3)),
    SubStrip(20, ledStrip + _LED_SUB_OFFSET(4)),
};

static const TstConfig AnimationConfig[LED_SUBSTRIP_NB] = {
//   Animation            Period    Offset  Speed   Direction                   Palette
    {SubStrip::CHECKERED, 2000,     0,      3,      SubStrip::FORWARD_INOUT,    pMyColorPalette2},
    {SubStrip::CHECKERED, 2000,     5,      3,      SubStrip::FORWARD_INOUT,    pMyColorPalette2},
    {SubStrip::CHECKERED, 2000,     10,     3,      SubStrip::FORWARD_INOUT,    pMyColorPalette2},
    {SubStrip::CHECKERED, 2000,     15,     3,      SubStrip::FORWARD_INOUT,    pMyColorPalette2},
    {SubStrip::CHECKERED, 2000,     20,     3,      SubStrip::FORWARD_INOUT,    pMyColorPalette2},
};

#if APP_TASKS
void vAppLedsTask(void *pvParam);
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
    for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
        pObj->eSetOffset(pstConfig->u8Offset);
        if (pObj->eSetAnimation(pstConfig->eAnimation, pstConfig->pPalette, pstConfig->u32Period, pstConfig->u8Speed) < SubStrip::RET_OK) {
            Serial.printf("[AppLED_init] SubStrip %u initialization failed\r\n", i);
        }
        pstConfig++;
        pObj++;
    }
#if APP_TASKS
    xTaskCreate(vAppLedsTask, LED_TASK, LED_TASK_HEAP, LED_TASK_PARAM, LED_TASK_PRIO, LED_TASK_HANDLE);
#endif
}

#if APP_TASKS
/*******************************************************************************
 * @brief AppLeds main task
 * 
 ******************************************************************************/
void vAppLedsTask(void *pvParam) {
    uint8_t u8Toggle = 0;
    uint8_t u8SubIndex = 0;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t u32Now;
    uint16_t u16Cnt = 0;
    SubStrip *pObj;
    while (1) {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(_LED_TIMEOUT));
        u32Now = millis();
        pObj = SubStrips;
        for (u8SubIndex = 0; u8SubIndex < LED_SUBSTRIP_NB; u8SubIndex++) {
            pObj->vManageAnimation(u32Now);
            pObj++;
        }
        FastLED.show();
        if ((u16Cnt % _LOOP_CNT_MS(1000)) == 0) {
            u16Cnt = 0;
            Serial.printf("[vAppLedsTask] Current tick: %u\r\n", millis());
        }
        u16Cnt++;
    }
}
#else
/*******************************************************************************
 * @brief Self-timed ledstrip refresh
 * 
 ******************************************************************************/
void AppLED_showLoop(void) {
    static uint32_t u32Timeout = 0;
    static uint32_t u32ColorFlipTimeout = 0;
    static uint32_t u32ShiftedTimeout = 0;
    uint32_t u32RightNow = millis();
    static uint8_t u8Toggle = 0;
    CRGB* Colors;

    if (u32ColorFlipTimeout < u32RightNow) {
        u32ColorFlipTimeout = u32RightNow + 2000;
        u8Toggle ^= 1;
        Colors = u8Toggle ? pMyColorPalette1 : pMyColorPalette2;
         for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
            SubStrips[i].vSetColorPalette(Colors);
        }
    }

    if (u32Timeout < u32RightNow) {
        u32Timeout = u32RightNow + _LED_TIMEOUT;
        for (uint8_t i = 0; i < LED_SUBSTRIP_NB; i++) {
            SubStrips[i].vManageAnimation(u32RightNow);
        }
        FastLED.show();
    }
}
#endif

#endif // APP_FASTLED

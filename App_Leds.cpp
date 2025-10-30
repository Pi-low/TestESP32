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
#define LED_TASK            "APP_LEDS"
#define LED_TASK_HEAP       4096 //(configMINIMAL_STACK_SIZE*4)
#define LED_TASK_PARAM      NULL
#define LED_TASK_PRIO       2
#define LED_TASK_HANDLE     NULL
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
SemaphoreHandle_t xLedStripSema;
void vAppLedsTask(void *pvParam);
void vAppLedsAnimTask(void *pvParam);
#endif

/*******************************************************************************
 * @brief Initialize ledstrip
 * 
 ******************************************************************************/
void AppLED_init(void) {
    char tcDbgString[PRINT_UTILS_MAX_BUF] = {0};
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
            snprintf(tcDbgString, PRINT_UTILS_MAX_BUF, "[AppLED_init] SubStrip %u set animation failed\r\n", i);
            APP_TRACE(tcDbgString);
        }
        pstConfig++;
        pObj++;
    }
#if APP_TASKS
    xTaskCreate(vAppLedsTask, LED_TASK, LED_TASK_HEAP, LED_TASK_PARAM, LED_TASK_PRIO, LED_TASK_HANDLE);
    xTaskCreate(vAppLedsAnimTask, "APP_ANIM", 1024, LED_TASK_PARAM, LED_TASK_PRIO, LED_TASK_HANDLE);
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
    xLedStripSema = xSemaphoreCreateBinary();
    uint32_t u32Now;
    uint16_t u16Cnt = 0;
    SubStrip *pObj;
    char pcString[PRINT_UTILS_MAX_BUF] = {0};
    xSemaphoreGive(xLedStripSema);
    while (1) {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(_LED_TIMEOUT));
        if (xLedStripSema)
        {
            if (xSemaphoreTake(xLedStripSema, pdMS_TO_TICKS(_LED_TIMEOUT/2))) {

                // protected ressource >>>
                u32Now = millis();
                pObj = SubStrips;
                for (u8SubIndex = 0; u8SubIndex < LED_SUBSTRIP_NB; u8SubIndex++) {
                    pObj->vManageAnimation(u32Now);
                    pObj++;
                }
                FastLED.show();
                // <<< end of protected ressource

                if (xSemaphoreGive(xLedStripSema) != pdTRUE) {
                    // Serial.println("[vAppLedsTask] xLedStripSema give error");
                    APP_TRACE("[vAppLedsTask] xLedStripSema give error\r\n");
                }
            }
        }
        
        if ((u16Cnt % _LOOP_CNT_MS(1000)) == 0) {
            u16Cnt = 0;
            snprintf(pcString, PRINT_UTILS_MAX_BUF, "[vAppLedsTask] Current tick: %u, xLedStripSema: %X\r\n", millis(), xLedStripSema);
            APP_TRACE(pcString);
        }
        u16Cnt++;
    }
}

void vAppLedsAnimTask(void *pvParam) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    SubStrip *pObj;
    while (1) {
        vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(LED_CHANGE_DELAY));
        if (xLedStripSema)
        {
            if( xSemaphoreTake(xLedStripSema, pdMS_TO_TICKS(1000)))
            {
                // protected ressource >>>
                APP_TRACE("[vAppLedsAnimTask] Changing animations\r\n");

                // <<< end of protected ressource
                if (xSemaphoreGive(xLedStripSema) != pdTRUE) {
                    // Serial.println("[vAppLedsAnimTask] xLedStripSema give error");
                    APP_TRACE("[vAppLedsAnimTask] xLedStripSema give error");
                }
            }
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

#endif // APP_FASTLED

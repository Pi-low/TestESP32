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
    CRGB* pSubstripAssemly;
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

typedef enum {
    LEDSTRIP_BLACKOUT,
    LEDSTRIP_STANDBY,
    LEDSTRIP_RUN,
    LEDSTRIP_FIXED,
} TeAppLED_LedstripStates;

/*******************************************************************************
 *  GLOBAL VARIABLES
 ******************************************************************************/
static TstStripCfg stAppLED_Config  = {0, 0, nullptr, nullptr, nullptr};
static CRGB pMyColorPalette1[3] = {CRGB::White, CRGB::Red, CRGB::Black};
static CRGB tCustomPalettes[LED_SUBSTRIP_NB][LED_STATIC_PALETTE_NB + 1] = {{{CRGB::Black}}};
static TeAppLED_LedstripStates eAppLed_CurrentState = LEDSTRIP_BLACKOUT;

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
uint8_t u8StrList2Index(const char* pcToSearch, const char **pcStrList, uint8_t u8LstSize);

/*******************************************************************************
 * @brief Initialize ledstrip
 * 
 ******************************************************************************/
void AppLED_init(void) {
    char tcPrint[PRINT_UTILS_MAX_BUF];
    bool bStartTasking = false;
    JsonArray jStrips;
    bAppCfg_LockJson();
    jStrips = jAppCfg_Config["DEVICE_SUBSTRIPS"].as<JsonArray>();
    stAppLED_Config.u8NbStrips = jStrips.size();
    if (stAppLED_Config.u8NbStrips)
    {
        stAppLED_Config.pu8Strips = (uint8_t*)pvPortMalloc(stAppLED_Config.u8NbStrips * sizeof(uint8_t));
        if (stAppLED_Config.pu8Strips)
        {
            uint8_t *ptr = stAppLED_Config.pu8Strips;
            for (uint8_t u8Val : jStrips)
            {
                *ptr = u8Val;
                stAppLED_Config.u16NbLeds += u8Val;
                ptr++;
            }
        }
        else 
        { APP_TRACE("[AppLED_init] Malloc error !\r\n"); }
    }
    bAppCfg_UnlockJson();

    if (stAppLED_Config.u16NbLeds && stAppLED_Config.u8NbStrips && stAppLED_Config.pu8Strips)
    {
        stAppLED_Config.pLedStrip = (CRGB*)pvPortMalloc(stAppLED_Config.u16NbLeds * sizeof(CRGB)); // Dynamic allocation, main display
        stAppLED_Config.pSubstripAssemly = (CRGB*)pvPortMalloc(stAppLED_Config.u16NbLeds * sizeof(CRGB)); // Dynamic allocation, fx generator
        stAppLED_Config.SubStrips = (SubStrip*)pvPortMalloc(stAppLED_Config.u8NbStrips * sizeof(SubStrip)); // Dynamic allocation
        if ((stAppLED_Config.pLedStrip != nullptr) && (stAppLED_Config.SubStrips != nullptr))
        {
            snprintf(tcPrint, PRINT_UTILS_MAX_BUF, "[AppLED_init] Loading %u strips:", stAppLED_Config.u8NbStrips);
            CRGB *pSub = stAppLED_Config.pSubstripAssemly;
            for (uint8_t u8cnt = 0; u8cnt < stAppLED_Config.u8NbStrips; u8cnt++)
            {
                stAppLED_Config.SubStrips[u8cnt] = SubStrip(stAppLED_Config.pu8Strips[u8cnt], pSub);
                snprintf(tcPrint + strlen(tcPrint), PRINT_UTILS_MAX_BUF - strlen(tcPrint), " %u", stAppLED_Config.pu8Strips[u8cnt]);
                pSub += stAppLED_Config.pu8Strips[u8cnt];
            }
            snprintf(tcPrint + strlen(tcPrint), PRINT_UTILS_MAX_BUF - strlen(tcPrint), "\r\nTotal ledstrip: %u\r\n", stAppLED_Config.u16NbLeds);
            APP_TRACE(tcPrint);
            ledStrip = stAppLED_Config.pLedStrip;
            SubStrips = stAppLED_Config.SubStrips;
            FastLED.addLeds<LED_CHIPSET, LED_DATA_PIN, LED_PIXEL_ORDER>(ledStrip, stAppLED_Config.u16NbLeds);
            FastLED.setBrightness(LED_BRIGHTNESS);
            FastLED.setCorrection(TypicalLEDStrip);
            FastLED.clear();
            FastLED.show();
            TstConfig *pstConfig = (TstConfig *)AnimationConfig;
            SubStrip *pObj = SubStrips;
            for (uint8_t i = 0; i < stAppLED_Config.u8NbStrips; i++)
            {
                pObj->eSetOffset(pstConfig->u8Offset);
                pObj->eSetDirection(pstConfig->eDirection);
                pObj->eSetFadeRate(pstConfig->u16MsFade);
                if (pObj->eSetAnimation(pstConfig->eAnimation, pstConfig->pPalette, pstConfig->u32Period, pstConfig->u8Speed) < SubStrip::RET_OK)
                {
                    snprintf(tcPrint, PRINT_UTILS_MAX_BUF, "[AppLED_init] SubStrip %u set animation failed\r\n", i);
                    APP_TRACE(tcPrint);
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
            snprintf(tcPrint, PRINT_UTILS_MAX_BUF, "[AppLED_init] Run task!\r\nFree heap: %u\r\n", ESP.getFreeHeap());
            APP_TRACE(tcPrint);
        }
    }
#endif
}

/*******************************************************************************
 * @brief AppLeds main task
 * 
 ******************************************************************************/
void vAppLedsTask(void *pvParam)
{
    char tcPrint[PRINT_UTILS_MAX_BUF] = {0};
    TickType_t xLastWakeTime = xTaskGetTickCount();
    TickType_t xTaskPeriod = pdMS_TO_TICKS(_LED_TIMEOUT);
    uint32_t u32Now;
    while (1)
    {
        switch (eAppLed_CurrentState)
        {
        case LEDSTRIP_BLACKOUT:
            xTaskPeriod = pdMS_TO_TICKS(100);
            FastLED.clear();
            FastLED.show();
            break;

        case LEDSTRIP_STANDBY:
            xTaskPeriod = pdMS_TO_TICKS(100);
            // freeze ledstrip
            break;

        case LEDSTRIP_RUN:
        if (LOCK_LEDS())
        {
            xTaskPeriod = pdMS_TO_TICKS(_LED_TIMEOUT); //update task period
            u32Now = millis();
            SubStrip *pObj = SubStrips;
            // manage substrip operation
            for (uint8_t u8Sub = 0; u8Sub < stAppLED_Config.u8NbStrips; u8Sub++)
            {
                pObj->vManageAnimation(u32Now);
                pObj++;
            }
            memcpy(ledStrip, stAppLED_Config.pSubstripAssemly, stAppLED_Config.u16NbLeds * sizeof(CRGB));
            FastLED.show();
            UNLOCK_LEDS();
        }
        break;

        case LEDSTRIP_FIXED:
            break;

        default:
            break;
        }
        vTaskDelayUntil(&xLastWakeTime, xTaskPeriod);
    } // end task loop
}

/*******************************************************************************
 * @brief AppLeds animation change task
 * 
 ******************************************************************************/
// void vAppLedsAnimTask(void *pvParam) {
//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     SubStrip *pObj;
//     char tcDbgString[PRINT_UTILS_MAX_BUF] = {0};
//     while (1) {
//         vTaskDelayUntil( &xLastWakeTime, pdMS_TO_TICKS(LED_CHANGE_DELAY));

//         if(LOCK_LEDS()) {
//             // protected ressource >>>
//             snprintf(tcDbgString, PRINT_UTILS_MAX_BUF, "[APP_ANIM] [%u] Animation event\r\n", millis());
//             APP_TRACE(tcDbgString);
//             // <<< end of protected ressource
//             UNLOCK_LEDS();
//         }
//     }
// }

void pvAppLed_CallbackEvent(std::string Payload) {
    JsonDocument jDoc;
    std::string strPrint;
    if (deserializeJson(jDoc, Payload) != DeserializationError::Ok)
    {
        strPrint = "Incoming event: ------>\r\n" + Payload + "\r\n<------\r\n";
    }
    else if (jDoc["id"].as<std::string>() != std::string(pcAppCfg_GetDeviceName()))
    {
        std::string Serialized;
        serializeJsonPretty(jDoc, Serialized);
        strPrint = "Incoming event: ------>\r\n" + Serialized + "\r\n<------\r\n";
    }
    if (!strPrint.empty())
        APP_TRACE(strPrint.c_str());
}

void pvAppLed_CallbackCmd(std::string Payload) {
    JsonDocument jDoc;
    std::string strPrint;
    if (deserializeJson(jDoc, Payload) != DeserializationError::Ok)
    {
        strPrint = "Incoming cmd: ------>\r\n" + Payload + "\r\n<------\r\n";
    }
    else if (jDoc["id"].as<std::string>() != std::string(pcAppCfg_GetDeviceName()))
    {
        std::string Serialized;
        serializeJsonPretty(jDoc, Serialized);
        strPrint = "Incoming cmd: ------>\r\n" + Serialized + "\r\n<------\r\n";
    }
    if (!strPrint.empty())
        APP_TRACE(strPrint.c_str());
}

eApp_RetVal eAppLed_blackout(void) {
    eAppLed_CurrentState = LEDSTRIP_BLACKOUT;
    return eRet_Ok;
}

eApp_RetVal eAppLed_resume(void) {
    eAppLed_CurrentState = LEDSTRIP_RUN;
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

eApp_RetVal eAppLed_ConfigSubstrip(uint8_t u8StripId, uint8_t u8CmdIndex, const char* pcValue)
{
    eApp_RetVal eRet;
    uint16_t u16value = atoi(pcValue);
    switch(u8CmdIndex)
    {
        case eArg_palette:
        eRet = eAppLed_SetPalette(u16value, u8StripId);
        break;

        case eArg_anim:
        eRet = eAppLed_SetAnimation((SubStrip::TeAnimation)u8StrList2Index(pcValue, (const char**)tpcAppLED_Animations, ARRAY_SIZEOF(tpcAppLED_Animations)), u8StripId);
        break;

        case eArg_speed:
        eRet = eAppLed_SetSpeed(u16value, u8StripId);
        break;

        case eArg_period:
        eRet = eAppLed_SetPeriod(u16value, u8StripId);
        break;

        case eArg_fade:
        eRet = eAppLed_SetFade(u16value, u8StripId);
        break;

        case eArg_dir:
        eRet = eAppLed_SetDirection((SubStrip::TeDirection)u16value, u8StripId);
        break;

        case eArg_offset:
        eRet = eAppLed_SetOffset(u16value, u8StripId);
        break;

        case eArg_bpm:
        eRet = eAppLed_SetBpm(u16value, u8StripId);
        break;
    }
    return eRet;
}

uint8_t u8StrList2Index(const char* pcToSearch, const char **pcStrList, uint8_t u8LstSize)
{
    char **ptrLst = (char**)pcStrList;
    uint8_t u8Ret = 0xFF;
    for (uint8_t u8Cnt = 0; u8Cnt < u8LstSize; u8Cnt++)
    {
        if (strncmp(pcToSearch, *ptrLst, strlen(pcToSearch)) == 0)
        {
            u8Ret = u8Cnt;
            break;
        }
        ptrLst++;
    }
    return u8Ret;
}

#endif // APP_FASTLED

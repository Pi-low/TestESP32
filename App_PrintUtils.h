/**
 * @brief Thread safe printing management
 * @file App_PrintUtils.h
 * @version 0.1
 * @date 2025-10-30
 * @author Nello
 */

#ifndef _APP_PRINT_UTILS_
#define _APP_PRINT_UTILS_

#include "Config.h"

#if defined(APP_TASKS) && APP_TASKS

#define PRINT_UTILS_MAX_BUF 256
#define APP_TRACE(x)     vAppPrintUtils_Print(x, strlen(x))
void vAppPrintUtils_init(void);
void vAppPrintUtils_Print(const char* pcDataToPrint, BaseType_t xLength);

#endif

#endif // _APP_PRINT_UTILS_

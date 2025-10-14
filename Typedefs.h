/**
 * @brief Types, enum, definitions...
 * @file Typedefs.h
 * @version 0.1
 * @date 2025-10-14
 * @author Nello
 */

#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef enum eApp_RetVal{
    eRet_Warning = 1,
    eRet_Ok = 0,
    eRet_Error = -1,
    eRet_Wait = -2,
} eApp_RetVal;

#endif // _TYPEDEFS_H

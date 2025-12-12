/**
 * @brief Types, enum, definitions...
 * @file Typedefs.h
 * @version 0.1
 * @date 2025-10-14
 * @author Nello
 */

#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H

#define GENERATE_STR(ENUM)      #ENUM,
#define ARRAY_SIZEOF(tab)       (sizeof(tab)/sizeof(*tab))
#define _MNG_RETURN(x)          eRet = x

typedef enum eApp_RetVal{
    eRet_Warning        = 1,
    eRet_Ok             = 0,
    eRet_Error          = -1,
    eRet_BadParameter   = eRet_Error - 1,
    eRet_InternalError  = eRet_Error - 2,
    eRet_JsonError      = eRet_Error - 3,
    eRet_NullPointer    = eRet_Error - 4,
} eApp_RetVal;

#endif // _TYPEDEFS_H

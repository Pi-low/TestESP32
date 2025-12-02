/**
 * @brief Types, enum, definitions...
 * @file Typedefs.h
 * @version 0.1
 * @date 2025-10-14
 * @author Nello
 */

#ifndef _TYPEDEFS_H
#define _TYPEDEFS_H

typedef enum eApp_RetVal{
    eRet_Warning        = 1,
    eRet_Ok             = 0,
    eRet_Error          = -1,
    eRet_BadParameter   = eRet_Error - 1,
    eRet_InternalError  = eRet_Error - 2,
    eRet_JsonError      = eRet_Error - 3,
} eApp_RetVal;

#endif // _TYPEDEFS_H

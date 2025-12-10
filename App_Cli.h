/**
 * @brief Command Line Interface engine
 * @file App_Cli.h
 * @version 0.1
 * @date 2025-11-04
 * @author Nello
 */

#ifndef _APP_CLI_H_
#define _APP_CLI_H_

#include <Arduino.h>
#include <SimpleCLI.h>
#include "Config.h"

#define GENERATE_ARG_ENUM(ENUM)         eArg_##ENUM,

#define FOREACH_SUBSTRIP_ARG(PARAM)     \
    PARAM(palette)                      \
    PARAM(anim)                         \
    PARAM(speed)                        \
    PARAM(period)                       \
    PARAM(fade)                         \
    PARAM(dir)                          \
    PARAM(offset)                       \
    PARAM(bpm)

#define FOREACH_SETMQTT_ARG(PARAM)      \
    PARAM(addr)                         \
    PARAM(port)                         \
    PARAM(login)                        \
    PARAM(pwd)                          \
    PARAM(topic)                        \
    PARAM(keepAlive)

#define FOREACH_SET_ARG(PARAM)          \
    PARAM(deviceName)                   \
    PARAM(strips)

enum eArgSubstrip{
    FOREACH_SUBSTRIP_ARG(GENERATE_ARG_ENUM)
};

enum eArgMqtt{
    FOREACH_SETMQTT_ARG(GENERATE_ARG_ENUM)
};

enum eArgSet{
    FOREACH_SET_ARG(GENERATE_ARG_ENUM)
};

void vAppCli_init(void);

#endif // _APP_CLI_H_

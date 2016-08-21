/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      app_debug.h
 *
 *  DESCRIPTION
 *      Header definitions for debug defines
 *
 ******************************************************************************/

#ifndef __APP_DEBUG_H__
#define __APP_DEBUG_H__

#ifdef DEBUG_ENABLE

#include <debug.h>

#define DEBUG_STR(s)  DebugWriteString(s)
#define DEBUG_U32(u)  DebugWriteUint32(u)
#define DEBUG_U16(u)  DebugWriteUint16(u)
#define DEBUG_U8(u)   DebugWriteUint8(u)
#define DEBUG_TEST_U8_ARR(x,offset,n)  do { \
    uint16 debug_len = offset; \
    while( debug_len < n) \
    { \
        DebugWriteUint8(x[debug_len]); \
        debug_len++; \
    } \
}while(0)
#else

#define DEBUG_STR(s)
#define DEBUG_U32(u)
#define DEBUG_U16(u)
#define DEBUG_U8(u)
#define DEBUG_TEST_U8_ARR(x,offset,n)
#endif /* DEBUG_ENABLE */

#endif /* __APP_DEBUG_H__ */
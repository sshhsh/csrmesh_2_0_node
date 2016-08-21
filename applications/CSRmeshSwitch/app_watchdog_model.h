/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      app_watchdog_model.h
 *
 *  DESCRIPTION
 *      Header definitions for Application Watchdog model functionality
 *
 *****************************************************************************/

#ifndef __APP_WATCHDOG_MODEL_H__
#define __APP_WATCHDOG_MODEL_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>
#include <bt_event_types.h>
#include <timer.h>

/*============================================================================*
 *  CSRmesh Header Files
 *============================================================================*/
#include <watchdog_server.h>
#include <csr_mesh.h>

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Default Watchdog Model Timing Parameters. */
#define DEFAULT_WDOG_INTERVAL           (5)
#define DEFAULT_WDOG_ACTIVE_AFTER       (1)

#if (DEFAULT_WDOG_INTERVAL < DEFAULT_WDOG_ACTIVE_AFTER)
#error "Watchdog Interval less than Active After duration"
#endif

#if (DEFAULT_WDOG_INTERVAL == 0)
#error "Watchdog Interval can't be zero."
#endif

/* Number of timers for watchdog model. */
#define WDOG_MODEL_TIMERS               (1)

/* Macros for NVM access */
#define NVM_WDOG_GROUP_DATA_OFFSET      (0)

#define NVM_OFFSET_WDOG_INTERVAL        (0)

#define NVM_OFFSET_WDOG_ACTIVE_AFTER    (1)

#define WDOG_MODEL_NVM_SIZE             (2)

/* CSRmesh watchdog model application data. */
typedef struct
{
    /* NVM Offset for Watchdog Model data. */
    uint16   nvm_offset;

    /* Timer ID for interval time. */
    timer_id second_tid;

    /* Count to keep track of interval time in seconds. */
    uint32   second_count;

    /* Watch dog model Inteval data */
    CSRMESH_WATCHDOG_INTERVAL_T interval;

    /* Model group ID list */
    uint16   groups[MAX_MODEL_GROUPS];

} APP_WATCHDOG_MODEL_DATA_T;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Initialize Application Watchdog Model Data.
 * Call this API before ReadPersistentStore().
 */
extern void AppWatchdogModelDataInit(void);

/* Initialize Watchdog Model. Call this API after CsrMeshInit(). */
extern void AppWatchdogModelInit(uint16 *groups, uint16 num_groups);

/* Reads Watchdog model Data from NVM. */
extern void AppWatchdogModelInitReadNVM(uint16 *p_offset);

/* Write Watchdog model Data from NVM. */
extern void AppWatchdogModelInitWriteNVM(uint16 *p_offset);

/* Watchdog model event handler. */
extern CSRmeshResult AppWatchdogEventHandler(CSRMESH_MODEL_EVENT_T event,
                                      CSRMESH_EVENT_DATA_T* msg,
                                      CsrUint16 len,
                                      void **state_data);

/* Start Application Watchdog. */
extern void AppWatchdogStart(void);

/* Stop application Watchdog. */
extern void AppWatchdogStop(void);

/* Pause Application Watchdog to Scan Continously. */
extern void AppWatchdogPause(void);

/* Reset the Application Watchdog data and NVM. */
extern void AppWatchdogResetState(void);

/* Write watchdog model data onto NVM */
extern void WdogWriteModelDataToNVM(void);

#endif /* __APP_WATCHDOG_MODEL_H__ */


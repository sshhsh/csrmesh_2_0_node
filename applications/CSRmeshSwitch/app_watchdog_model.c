/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      app_watchdog_model.c
 *
 *  DESCRIPTION
 *      This file defines routines for using the watchdog model in application.
 *
 *****************************************************************************/
/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <gatt.h>
#include <gatt_prim.h>
#include <mem.h>
#include <buf_utils.h>
#include <random.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"
#include "app_debug.h"
#include "nvm_access.h"
#include "app_watchdog_model.h"
#include "csr_mesh_switch.h"
#include "app_mesh_event_handler.h"

/*============================================================================*
 *  CSRmesh Header Files
 *============================================================================*/
#include <csr_mesh.h>

#ifdef ENABLE_WATCHDOG_MODEL
/*============================================================================*
 *  Private Data Types
 *============================================================================*/
/* Application Watchdog States. */
typedef enum
{
    app_wdog_stopped = 0,
    app_wdog_paused  = 1,
    app_wdog_running = 2
} APP_WDOG_STATE_T;

/*============================================================================*
 *  Private Data
 *============================================================================*/
/* Application Watchdog model data. */
static APP_WATCHDOG_MODEL_DATA_T app_wdog_data;

/* Current State of Application Watchdog. */
static APP_WDOG_STATE_T wdog_state;

/*============================================================================*
 *  Private Function Prototypes.
 *============================================================================*/
static void resetWatchdogState(void);
static void wdogReadModelDataFromNVM(void);
static void wdogIntervalTimeoutHandler(timer_id tid);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/
/*----------------------------------------------------------------------------*
 *  NAME
 *      resetWatchdogState
 *
 *  DESCRIPTION
 *      This function resets the watchdog state.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void resetWatchdogState(void)
{
    AppWatchdogStop();

    app_wdog_data.interval.activeaftertime = DEFAULT_WDOG_ACTIVE_AFTER;
    app_wdog_data.interval.interval     = DEFAULT_WDOG_INTERVAL;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      wdogReadModelDataFromNVM
 *
 *  DESCRIPTION
 *      This function reads the model data from NVM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void wdogReadModelDataFromNVM(void)
{
    Nvm_Read(&app_wdog_data.interval.interval, 1, app_wdog_data.nvm_offset +
                                                      NVM_OFFSET_WDOG_INTERVAL);
    Nvm_Read(&app_wdog_data.interval.activeaftertime, 1, 
                                                  NVM_OFFSET_WDOG_ACTIVE_AFTER);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      wdogIntervalTimeoutHandler
 *
 *  DESCRIPTION
 *      This function keeps track of interval and sends watchdog message
 *      at every interval.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void wdogIntervalTimeoutHandler(timer_id tid)
{
    CSRMESH_WATCHDOG_MESSAGE_T wdog_msg;

    if (tid == app_wdog_data.second_tid)
    {
        /* Increment Seconds Counter. */
        app_wdog_data.second_count++;

        /* Start timer for next second. */
        app_wdog_data.second_tid = TimerCreate(SECOND, TRUE,
                                                 wdogIntervalTimeoutHandler);

        /* Check if the interval number of seconds elapsed. */
        if (app_wdog_data.second_count >= app_wdog_data.interval.interval)
        {
            /* Send Watch dog Message if wdog state is running. */
            if (app_wdog_running == wdog_state)
            {
                uint8 idx;
                /* Generate random message and random length */
                wdog_msg.randomdata_len = (Random16())%sizeof(wdog_msg.randomdata);
                for (idx = 0; idx < wdog_msg.randomdata_len; idx++)
                {
                    wdog_msg.randomdata[idx] = (Random16()) & 0xFF;
                }

                /* Reset the interval counter. */
                app_wdog_data.second_count = 0;

                /* Start Listening if the active after interval is non-zero. */
                if (app_wdog_data.interval.activeaftertime)
                {
                    /* Start Listening to CSRmesh messages. */
                    EnableHighDutyScanMode(TRUE);
                    DEBUG_STR("WATCHDOG: LISTENING in high scan duty mode\r\n");
                }

                WatchdogMessage(CSR_MESH_DEFAULT_NETID, wdog_model_groups[0], 
                                &wdog_msg);
            }
        }
        else if (app_wdog_data.second_count >= 
                                         app_wdog_data.interval.activeaftertime)
        {
            /* If active_after duration number of seconds elapsed
             * and watchdog is running.
             */
            if (app_wdog_running == wdog_state)
            {
                EnableHighDutyScanMode(FALSE); 
                DEBUG_STR("WATCHDOG: LISTENING in low scan duty mode\r\n");
            }
        }
    }
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppWatchdogModelDataInit
 *
 *  DESCRIPTION
 *      This function initializes watchdog model data in application.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppWatchdogModelDataInit(void)
{
    resetWatchdogState();
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppWatchdogModelInit
 *
 *  DESCRIPTION
 *      This function initializes watchdog model.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppWatchdogModelInit(uint16 *groups, uint16 num_groups)
{
    WatchdogModelInit(CSR_MESH_DEFAULT_NETID, groups, 
                      num_groups, AppWatchdogEventHandler);
    
    AppWatchdogModelDataInit();
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppWatchdogModelInitReadNVM
 *
 *  DESCRIPTION
 *      This function reads data from NVM during initialization.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppWatchdogModelInitReadNVM(uint16 *p_offset)
{
    app_wdog_data.nvm_offset = *p_offset;

    wdogReadModelDataFromNVM();

    *p_offset += WDOG_MODEL_NVM_SIZE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppWatchdogModelInitWriteNVM
 *
 *  DESCRIPTION
 *      This function write data from NVM during initialization.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppWatchdogModelInitWriteNVM(uint16 *p_offset)
{
    app_wdog_data.nvm_offset = *p_offset;

    WdogWriteModelDataToNVM();

    *p_offset += WDOG_MODEL_NVM_SIZE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppWatchdogStart
 *
 *  DESCRIPTION
 *      This function starts application watchdog. Needs to be called after
 *      association completion.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppWatchdogStart(void)
{
    if (app_wdog_stopped == wdog_state)
    {
        /* Delete timer just to make sure things work properly
         * if start is called multiple times.
         */
        TimerDelete(app_wdog_data.second_tid);

        /* Start the interval timer. */
        app_wdog_data.second_tid = TimerCreate(SECOND, TRUE,
                                               wdogIntervalTimeoutHandler);
    }

    /* Set State to Running. */
    wdog_state = app_wdog_running;
    DEBUG_STR("WATCHDOG : RUNNING STATE\r\n");
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppWatchdogStop
 *
 *  DESCRIPTION
 *      This function stops application watchdog.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppWatchdogStop(void)
{
    TimerDelete(app_wdog_data.second_tid);
    app_wdog_data.second_tid   = TIMER_INVALID;
    app_wdog_data.second_count = 0;
    wdog_state = app_wdog_stopped;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppWatchdogPause
 *
 *  DESCRIPTION
 *      This function pauses application watchdog.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppWatchdogPause(void)
{
    wdog_state = app_wdog_paused;
    EnableHighDutyScanMode(TRUE);
    DEBUG_STR("WATCHDOG PAUSED : LISTENING in high scan duty mode\r\n");
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppWatchdogResetState
 *
 *  DESCRIPTION
 *      This function resets the state of watchdog model.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppWatchdogResetState(void)
{
    resetWatchdogState();

    WdogWriteModelDataToNVM();

    /* Start Listening. */
    EnableHighDutyScanMode(TRUE);
    DEBUG_STR("WATCHDOG RESET : LISTENING in high scan duty mode\r\n");
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      WdogWriteModelDataToNVM
 *
 *  DESCRIPTION
 *      This function writes the model data to NVM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void WdogWriteModelDataToNVM(void)
{
    Nvm_Write(&app_wdog_data.interval.interval, 
              sizeof(CsrUint16),
              app_wdog_data.nvm_offset + NVM_OFFSET_WDOG_INTERVAL);
    
    Nvm_Write(&app_wdog_data.interval.activeaftertime,
              sizeof(CsrUint16), 
              app_wdog_data.nvm_offset + NVM_OFFSET_WDOG_ACTIVE_AFTER);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppWatchdogModelHandler
 *
 *  DESCRIPTION
 *      This function handles watchdog model commands.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern CSRmeshResult AppWatchdogEventHandler(CSRMESH_MODEL_EVENT_T event,
                                      CSRMESH_EVENT_DATA_T* msg,
                                      CsrUint16 len,
                                      void **state_data)
{
    bool update_lastetag = FALSE;
    switch (event)
    {
        case CSRMESH_WATCHDOG_SET_INTERVAL:
        {
            CSRMESH_WATCHDOG_SET_INTERVAL_T *p_msg = 
                                  (CSRMESH_WATCHDOG_SET_INTERVAL_T *)msg->data;

            /* Check if interval is not zero and interval is greater
             * than active after duration.
             */
            if ((p_msg->interval > 0) && 
                (p_msg->interval >= p_msg->activeaftertime))
            {
                app_wdog_data.interval.interval = p_msg->interval;
                app_wdog_data.interval.activeaftertime = p_msg->activeaftertime;

                WdogWriteModelDataToNVM();

                update_lastetag = TRUE;
            }

            /* Check if state_data pointer is NULL. */
            if (state_data)
            {
                *state_data = (void *)&app_wdog_data.interval;
            }
        }
        break;

        default:
        break;
    }

    return update_lastetag;
}
#endif /* ENABLE_WATCHDOG_MODEL */

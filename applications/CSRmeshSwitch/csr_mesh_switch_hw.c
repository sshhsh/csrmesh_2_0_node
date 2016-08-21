/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      csr_mesh_switch_hw.c
 *
 *  DESCRIPTION
 *      This file implements the CSRmesh switch hardware specific functions.
 *
 *  NOTE
 *      Default hardware is always IOT board.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>
#include <pio.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"
#include "iot_hw.h"
#include "csr_mesh_switch_hw.h"
#include "nvm_access.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/
/* Time for which the button bounce is expected */
#define BUTTON_DEBOUNCE_TIME           (20 * MILLISECOND)

/* Timer to check if button was pressed for 1 second */
#define BUTTON_ONE_SEC_PRESS_TIME      (1 * SECOND)

#define KEY_PRESSED  (TRUE)

#define KEY_RELEASED (FALSE)

/* Light Level Increment/Decrement Step Size */
#define LEVEL_STEP_SIZE               (4)

/* Maximum and Minimum light levels */
#define MAX_LEVEL                     (255)
#define MIN_LEVEL                     (0)

/*============================================================================*
 *  Private Data
 *============================================================================*/
#ifndef DEBUG_ENABLE
/* Switch Button States. */
static bool     onButtonState  = KEY_RELEASED;
static bool     incButtonState = KEY_RELEASED;
static bool     decButtonState = KEY_RELEASED;
static timer_id oneSecTimerId  = TIMER_INVALID;

static uint8    switch_cmd_tid = 1;
#endif /* DEBUG_ENABLE */


/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/


/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/
#ifndef DEBUG_ENABLE
/*----------------------------------------------------------------------------*
 *  NAME
 *      handleButtonDebounce
 *
 *  DESCRIPTION
 *      This function handles De-bounce Timer Events.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void handleButtonDebounce(timer_id tid)
{
    bool startOneSecTimer = FALSE;
    bool update_nvm = FALSE;
    CSRMESH_POWER_SET_STATE_T power_state;
    CSRMESH_LIGHT_SET_LEVEL_T light_level;

    if( tid == g_switchapp_data.debounce_tid)
    {
        g_switchapp_data.debounce_tid = TIMER_INVALID;

        /* Enable PIO Events again */
        PioSetEventMask(BUTTONS_BIT_MASK, pio_event_mode_both);

        /* If PIO State is same as before starting de-bounce timer,
         * we have a valid event.
         */
        if ((PioGet(SW3_PIO) == FALSE) && (incButtonState == KEY_RELEASED))
        {
            /* Set State and increment level */
            incButtonState = KEY_PRESSED;
            if (g_switchapp_data.brightness_level 
                                            < (MAX_LEVEL - LEVEL_STEP_SIZE))
            {
                g_switchapp_data.brightness_level += LEVEL_STEP_SIZE;
            }
            else
            {
                g_switchapp_data.brightness_level = MAX_LEVEL;
            }

            /* Start 1 second timer */
            startOneSecTimer = TRUE;
        }
        else if ((PioGet(SW3_PIO) == TRUE) && (incButtonState == KEY_PRESSED))
        {
            /* Set state to KEY RELEASE */
            incButtonState = KEY_RELEASED;
            update_nvm = TRUE;
        }

        if ((PioGet(SW2_PIO) == FALSE) && (decButtonState == KEY_RELEASED))
        {
            /* Set State and decrement level */
            decButtonState = KEY_PRESSED;
            if (g_switchapp_data.brightness_level > LEVEL_STEP_SIZE)
            {
                g_switchapp_data.brightness_level -= LEVEL_STEP_SIZE;
            }
            else
            {
                g_switchapp_data.brightness_level = MIN_LEVEL;
            }

            /* Start 1 second timer */
            startOneSecTimer = TRUE;
        }
        else if ((PioGet(SW2_PIO) == TRUE) && (decButtonState == KEY_PRESSED))
        {
            /* Set state to KEY RELEASE */
            decButtonState = KEY_RELEASED;
            update_nvm = TRUE;
        }

        if ((PioGet(SW4_PIO) == FALSE) && (onButtonState == KEY_RELEASED))
        {
            /* Set Button State */
            onButtonState = KEY_PRESSED;
            power_state.state = csr_mesh_power_state_on;
            power_state.tid = switch_cmd_tid++;
            PowerSetState(0, switch_model_groups[0], &power_state, FALSE);
        }
        else if ((PioGet(SW4_PIO) == TRUE) && (onButtonState == KEY_PRESSED))
        {
            /* Set state to KEY RELEASE */
            onButtonState = KEY_RELEASED;
            power_state.state = csr_mesh_power_state_off;
            power_state.tid = switch_cmd_tid++;
            PowerSetState(0, switch_model_groups[0], &power_state, FALSE);
        }

        /* Send Light Command and Create One Second Timer when flag is set */
        if (startOneSecTimer)
        {
            light_level.level = g_switchapp_data.brightness_level;
            light_level.tid = switch_cmd_tid++;
            LightSetLevel(DEFAULT_NW_ID, switch_model_groups[0], &light_level, 
                                                                         FALSE);

            /* Start 1 second timer */
            oneSecTimerId = TimerCreate(BUTTON_ONE_SEC_PRESS_TIME, TRUE,
                                                    handleButtonDebounce);
        }
    }
    else if (tid == oneSecTimerId)
    {
        oneSecTimerId = TIMER_INVALID;

        /* Key has been held Pressed for a second now */
        if ((PioGet(SW3_PIO) == FALSE) && (incButtonState == KEY_PRESSED))
        {
            if (g_switchapp_data.brightness_level < 
                                            (MAX_LEVEL - (5*LEVEL_STEP_SIZE)))
            {
                g_switchapp_data.brightness_level += (5*LEVEL_STEP_SIZE);
            }
            else
            {
                g_switchapp_data.brightness_level = MAX_LEVEL;
            }

            light_level.level = g_switchapp_data.brightness_level;
            light_level.tid = switch_cmd_tid++;
            LightSetLevel(DEFAULT_NW_ID, switch_model_groups[0], &light_level, 
                                                                         FALSE);

            oneSecTimerId = TimerCreate(BUTTON_ONE_SEC_PRESS_TIME, TRUE,
                                                    handleButtonDebounce);
        }

        /* Key has been held Pressed for a second now */
        if ((PioGet(SW2_PIO) == FALSE) && (decButtonState == KEY_PRESSED))
        {
            if (g_switchapp_data.brightness_level > (5*LEVEL_STEP_SIZE))
            {
                g_switchapp_data.brightness_level -= (5*LEVEL_STEP_SIZE);
            }
            else
            {
                g_switchapp_data.brightness_level = MIN_LEVEL;
            }

            light_level.level = g_switchapp_data.brightness_level;
            light_level.tid = switch_cmd_tid++;
            LightSetLevel(DEFAULT_NW_ID, switch_model_groups[0], &light_level, 
                                                                         FALSE);

            oneSecTimerId = TimerCreate(BUTTON_ONE_SEC_PRESS_TIME, TRUE,
                                                    handleButtonDebounce);
        }
    }

    /* Restart the tid */
    switch_cmd_tid  = switch_cmd_tid > 255? 0: switch_cmd_tid;

    /* Update NVM if required */
    if (update_nvm)
    {
        Nvm_Write((uint16 *)&g_switchapp_data.brightness_level, 
                   sizeof(uint16),
                   NVM_OFFSET_SWITCH_STATE);
    }
}


/*============================================================================*
 *  Public function definitions
 *============================================================================*/
/*----------------------------------------------------------------------------*
 *  NAME
 *      SwitchHardwareInit
 *
 *  DESCRIPTION
 *      This function initializes the light hardware, like PIO, interface etc.
 *
 * PARAMETERS
 *      Nothing.
 *
 * RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void SwitchHardwareInit(void)
{
    IOTLightControlDeviceInit();
    IOTSwitchInit();
    /* Switch off the light */
    LightHardwarePowerControl(FALSE);

    /* Read the satus of the SW4 PIO and set the position accordingly */
    if(PioGet(SW4_PIO) == FALSE)
    {
        onButtonState = KEY_PRESSED;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandlePIOChangedEvent
 *
 *  DESCRIPTION
 *      This function handles PIO Changed event
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandlePIOChangedEvent(uint32 pio_changed)
{
    bool start_timer = FALSE;

    /* When switch is not associated in stand alone mode,
     * don't send any CSR Mesh events.
     */
    if (g_switchapp_data.assoc_state != app_state_associated)
    {
        return;
    }

    if (pio_changed & BUTTONS_BIT_MASK)
    {
        /* Check if PIO Changed from previous state */
        if (((PioGet(SW3_PIO) == FALSE) && (incButtonState == KEY_RELEASED)) ||
            ((PioGet(SW3_PIO) == TRUE) && (incButtonState == KEY_PRESSED)))
        {
            start_timer = TRUE;
        }

        if (((PioGet(SW2_PIO) == FALSE) && (decButtonState == KEY_RELEASED)) ||
            ((PioGet(SW2_PIO) == TRUE) && (decButtonState == KEY_PRESSED)))
        {
            start_timer = TRUE;
        }

        if (((PioGet(SW4_PIO) == FALSE) && (onButtonState== KEY_RELEASED)) ||
            ((PioGet(SW4_PIO) == TRUE) && (onButtonState == KEY_PRESSED)))
        {
            start_timer = TRUE;
        }
    }

    if (start_timer)
    {
        /* Disable further Events */
        PioSetEventMask(BUTTONS_BIT_MASK, pio_event_mode_disable);

        /* Start a timer for de-bounce and delete one second timer
         * as we received a new event.
         */
        TimerDelete(oneSecTimerId);
        oneSecTimerId = TIMER_INVALID;
        TimerDelete(g_switchapp_data.debounce_tid);
        g_switchapp_data.debounce_tid =
                TimerCreate(BUTTON_DEBOUNCE_TIME, TRUE, handleButtonDebounce);
    }
}
#endif /* DEBUG_ENABLE */

/*----------------------------------------------------------------------------*
 *  NAME
 *      LightHardwarePowerControl
 *
 *  DESCRIPTION
 *      Controls the light device power.
 *
 * PARAMETERS
 *      power_on [in] Turns ON power if TRUE.
 *                    Turns OFF power if FALSE.
 *
 * RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void LightHardwarePowerControl(bool power_on)
{
    IOTLightControlDevicePower(power_on);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      LightHardwareSetColor
 *
 *  DESCRIPTION
 *      Controls the color of the light.
 *
 * PARAMETERS
 *      red   [in] 0-255 levels of Red colour component.
 *      green [in] 0-255 levels of Green colour component.
 *      blue  [in] 0-255 levels of Blue colour component.
 *
 * RETURNS
 *      TRUE  if set color is supported by device.
 *      FALSE if it device is a white light or mono-chromatic light.
 *
 *----------------------------------------------------------------------------*/
extern bool LightHardwareSetColor(uint8 red, uint8 green, uint8 blue)
{
    IOTLightControlDeviceSetColor(red, green, blue);
    return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      LightHardwareSetLevel
 *
 *  DESCRIPTION
 *      Controls the brightness of the White light.
 *
 * PARAMETERS
 *      red   [in] 0-255 levels of intensity.
 *
 * RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void LightHardwareSetLevel(uint8 brightness)
{
    IOTLightControlDeviceSetLevel(brightness);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      LightHardwareSetBlink
 *
 *  DESCRIPTION
 *      Controls the blink colour and duration of light.
 *
 * PARAMETERS
 *      red      [in] 0-255 levels of Red colour component.
 *      green    [in] 0-255 levels of Green colour component.
 *      blue     [in] 0-255 levels of Blue colour component.
 *      on_time  [in] ON duration in multiples of 16ms.
 *      off_time [in] OFF duration in multiples of 16ms.
 *
 * RETURNS
 *      TRUE  if light blink is supported by device.
 *      FALSE if light blink is NOT supported.
 *
 *----------------------------------------------------------------------------*/
extern bool LightHardwareSetBlink(uint8 red, uint8 green, uint8 blue,
                                  uint8 on_time, uint8 off_time)
{
    IOTLightControlDeviceBlink(red, green, blue, on_time, off_time);
    return TRUE;
}


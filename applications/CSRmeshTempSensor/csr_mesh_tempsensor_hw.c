/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      tempsensor_hw.c
 *
 *  DESCRIPTION
 *      This file implements the abstraction for temperature sensor hardware
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <pio.h>
#include <timer.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"
#include "csr_mesh_tempsensor_hw.h"
#include "csr_mesh_tempsensor.h"
#ifdef TEMPERATURE_SENSOR_STTS751
#include "stts751_temperature_sensor.h"
#endif /* TEMPERATURE_SENSOR_STTS751 */
#include "csr_mesh_tempsensor_util.h"
#include "app_debug.h"
#include "iot_hw.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/
/* Time for which the button bounce is expected */
#define BUTTON_DEBOUNCE_TIME                (20 * MILLISECOND)

/* Timer to check if button was pressed for 1 second */
#define BUTTON_ONE_SEC_PRESS_TIME           (1 * SECOND)

/* Button and switch states */
#define KEY_PRESSED                         (TRUE)
#define KEY_RELEASED                        (FALSE)

/* Increment/Decrement Step for Button Press. */
#define STEP_SIZE_PER_BUTTON_PRESS          (32)

/* Macro to convert Celsius to 1/32 kelvin units. */
#define CELSIUS_TO_BY32_KELVIN(x)          (((x)*32) + CELSIUS_TO_KELVIN_FACTOR)

/* Max. Temperature. */
#define MAX_DESIRED_TEMPERATURE             (CELSIUS_TO_BY32_KELVIN(40))

/* Min. Temperature. */
#define MIN_DESIRED_TEMPERATURE             (CELSIUS_TO_BY32_KELVIN(-5))

/*============================================================================*
 *  Private Data
 *============================================================================*/
/* Temperature sensor read delay after issuing read command. */
#ifdef TEMPERATURE_SENSOR_STTS751
#define TEMP_READ_DELAY         (MAX_CONVERSION_TIME * MILLISECOND)
#endif /* TEMPERATURE_SENSOR_STTS751 */

/* Event handler to be called after temperature is read from sensor. */
static TEMPSENSOR_EVENT_HANDLER_T eventHandler;

/* Timer ID for temperature read delay. */
static timer_id read_delay_tid = TIMER_INVALID;

#ifdef ENABLE_TEMPERATURE_CONTROLLER
/* Temperature Controller Button Debounce Timer ID. */
static timer_id oneSecTimerId = TIMER_INVALID;

#ifndef DEBUG_ENABLE
/* Toggle switch de-bounce timer id */
static timer_id debounce_tid = TIMER_INVALID;

/* Switch Button States. */
static bool     incButtonState = KEY_RELEASED;
static bool     decButtonState = KEY_RELEASED;
#endif /* DEBUG_ENABLE */
#endif /* ENABLE_TEMPERATURE_CONTROLLER */

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/
/*----------------------------------------------------------------------------*
 *  NAME
 *      tempSensorReadyToRead
 *
 *  DESCRIPTION
 *      This function is called after a duration of temperature read delay,
 *      once a read is initiated.
 *
 *  RETURNS
 *      TRUE if initialization is sucessful.
 *
 *----------------------------------------------------------------------------*/
#ifdef TEMPERATURE_SENSOR_STTS751
static void tempSensorReadyToRead(timer_id tid)
{
    int16 temp;

    if (tid == read_delay_tid)
    {
        read_delay_tid = TIMER_INVALID;

        /* Read the temperature. */
        STTS751_ReadTemperature(&temp);
        if (temp != INVALID_TEMPERATURE)
        {
            /* Convert temperature in to 1/32 degree Centigrade units */
            temp = (temp << 1);

            temp += CELSIUS_TO_KELVIN_FACTOR;
        }

        /* Report the temperature read. */
        eventHandler(temp);
    }
}
#endif /* TEMPERATURE_SENSOR_STTS751 */


#ifdef ENABLE_TEMPERATURE_CONTROLLER
#ifdef DEBUG_ENABLE
/*----------------------------------------------------------------------------*
 *  NAME
 *      handleDesiredTempChange
 *
 *  DESCRIPTION
 *      This function handles the case when desired temperature is changed 
 *      through uart.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleDesiredTempChange(timer_id tid)
{
    if (tid == oneSecTimerId)
    {
        oneSecTimerId = TIMER_INVALID;
        /* Check if the current desired value of temperature is,
         * different from last broadcast value.
         */
        if (current_desired_air_temp != last_bcast_desired_air_temp)
        {
            /* Set last Broadcast Temperature to current temperature. */
            last_bcast_desired_air_temp = current_desired_air_temp;

            StartTempTransmission();
        }

        /* Display Temperature. */

        DEBUG_STR(" USER CHANGED DESIRED TEMPERATURE : ");
        PrintInDecimal(current_desired_air_temp/32);
        DEBUG_STR(" kelvin\r\n");
    }
}

/*----------------------------------------------------------------------------*
 * NAME
 *    UartDataRxCallback
 *
 * DESCRIPTION
 *     This callback is issued when data is received over UART. Application
 *     may ignore the data, if not required. For more information refer to
 *     the API documentation for the type "uart_data_out_fn"
 *
 * RETURNS
 *     The number of words processed, return data_count if all of the received
 *     data had been processed (or if application don't care about the data)
 *
 *----------------------------------------------------------------------------*/
extern uint16 UartDataRxCallback ( void* p_data, uint16 data_count,
        uint16* p_num_additional_words )
{
    bool change = FALSE;
    uint8 *byte = (uint8 *)p_data;

    /* Application needs 1 additional data to be received */
    *p_num_additional_words = 1;

    /* If device is not associated, return. */
    if (g_tsapp_data.assoc_state != app_state_associated)
    {
        return data_count;
    }

    switch(byte[0])
    {
        case '+':
        {
            if (current_desired_air_temp < 
                (MAX_DESIRED_TEMPERATURE - STEP_SIZE_PER_BUTTON_PRESS) &&
                IsSensorConfigured())
            {
                change = TRUE;
                current_desired_air_temp += STEP_SIZE_PER_BUTTON_PRESS;
                DEBUG_STR("+ 1 kelvin\r\n");
            }
        }
        break;

        case '-':
        {
            if (current_desired_air_temp > 
                (MIN_DESIRED_TEMPERATURE + STEP_SIZE_PER_BUTTON_PRESS) &&
                IsSensorConfigured())
            {
                change = TRUE;
                current_desired_air_temp -= STEP_SIZE_PER_BUTTON_PRESS;
                DEBUG_STR("- 1 kelvin\r\n");
            }
        }
        break;

        default:
        break;
    }

    if (change)
    {
        TimerDelete(oneSecTimerId);
        oneSecTimerId = TimerCreate(BUTTON_ONE_SEC_PRESS_TIME, TRUE,
                                          handleDesiredTempChange);
    }

    return data_count;
}

#else /* DEBUG_ENABLE */

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

    if( tid == debounce_tid)
    {
        debounce_tid = TIMER_INVALID;

        /* Enable PIO Events again */
        PioSetEventMask(BUTTONS_BIT_MASK, pio_event_mode_both);

        /* If PIO State is same as before starting de-bounce timer,
         * we have a valid event.
         */
        if ((PioGet(SW3_PIO) == FALSE) && (incButtonState == KEY_RELEASED))
        {
            /* Set State and increment level */
            incButtonState = KEY_PRESSED;
            if(IsSensorConfigured())
            {
                if (current_desired_air_temp < 
                        (MAX_DESIRED_TEMPERATURE - STEP_SIZE_PER_BUTTON_PRESS))
                    current_desired_air_temp += STEP_SIZE_PER_BUTTON_PRESS;
                else
                    current_desired_air_temp = MAX_DESIRED_TEMPERATURE;
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
            if(IsSensorConfigured())
            {
                if (current_desired_air_temp > 
                        (MIN_DESIRED_TEMPERATURE - STEP_SIZE_PER_BUTTON_PRESS))
                    current_desired_air_temp -= STEP_SIZE_PER_BUTTON_PRESS;
                else
                    current_desired_air_temp = MIN_DESIRED_TEMPERATURE;
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


        /* Create One Second Timer when flag is set */
        if (startOneSecTimer)
        {
            if(IsSensorConfigured())
            {
                if (current_desired_air_temp != last_bcast_desired_air_temp)
                {
                    /* Set last Broadcast Temperature to current temperature. */
                    last_bcast_desired_air_temp = current_desired_air_temp;

                    /* Send New desired temperature. */
                    StartTempTransmission();
                }
            }
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
            if(IsSensorConfigured())
            {
                if (current_desired_air_temp < 
                       (MAX_DESIRED_TEMPERATURE - (STEP_SIZE_PER_BUTTON_PRESS)))
                    current_desired_air_temp += (STEP_SIZE_PER_BUTTON_PRESS);
                else
                    current_desired_air_temp = MAX_DESIRED_TEMPERATURE;

                if (current_desired_air_temp != last_bcast_desired_air_temp)
                {
                    /* Set last Broadcast Temperature to current temperature. */
                    last_bcast_desired_air_temp = current_desired_air_temp;

                    /* Send New desired temperature. */
                    StartTempTransmission();
                }
            }
            oneSecTimerId = TimerCreate(BUTTON_ONE_SEC_PRESS_TIME, TRUE,
                                                    handleButtonDebounce);
        }

        /* Key has been held Pressed for a second now */
        if ((PioGet(SW2_PIO) == FALSE) && (decButtonState == KEY_PRESSED))
        {
            if(IsSensorConfigured())
            {
                if (current_desired_air_temp > 
                       (MIN_DESIRED_TEMPERATURE - (STEP_SIZE_PER_BUTTON_PRESS)))
                    current_desired_air_temp -= (STEP_SIZE_PER_BUTTON_PRESS);
                else
                    current_desired_air_temp = MIN_DESIRED_TEMPERATURE;

                if (current_desired_air_temp != last_bcast_desired_air_temp)
                {
                    /* Set last Broadcast Temperature to current temperature. */
                    last_bcast_desired_air_temp = current_desired_air_temp;

                    /* Send New desired temperature. */
                    StartTempTransmission();
                }
            }
            oneSecTimerId = TimerCreate(BUTTON_ONE_SEC_PRESS_TIME, TRUE,
                                                    handleButtonDebounce);
        }
    }
}
#endif /* DEBUG_ENABLE */
#endif /* ENABLE_TEMPERATURE_CONTROLLER */

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

#ifdef ENABLE_TEMPERATURE_CONTROLLER
#ifndef DEBUG_ENABLE
/*-----------------------------------------------------------------------------*
 *  NAME
 *      HandlePIOEvent
 *
 *  DESCRIPTION
 *      This function handles the PIO Events.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
extern void HandlePIOEvent(uint32 pio_changed)
{
    bool start_timer = FALSE;

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
        TimerDelete(debounce_tid);
        debounce_tid =
                TimerCreate(BUTTON_DEBOUNCE_TIME, TRUE, handleButtonDebounce);
    }
}
#endif /* DEBUG_ENABLE */
#endif /* ENABLE_TEMPERATURE_CONTROLLER */

/*----------------------------------------------------------------------------*
 *  NAME
 *      TempSensorHardwareInit
 *
 *  DESCRIPTION
 *      This function initialises the temperature sensor hardware.
 *
 *  RETURNS
 *      TRUE if initialization is sucessful.
 *
 *----------------------------------------------------------------------------*/
extern bool TempSensorHardwareInit(TEMPSENSOR_EVENT_HANDLER_T handler)
{
    bool status = FALSE;

    read_delay_tid = TIMER_INVALID;

    if (NULL != handler)
    {
        eventHandler = handler;
#ifdef TEMPERATURE_SENSOR_STTS751
        status = STTS751_Init();
#endif /* TEMPERATURE_SENSOR_STTS751 */
    }

    return status;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      TempSensorRead
 *
 *  DESCRIPTION
 *      This function sends a temperature read command to the sensor.
 *      Temperature will be reported in the registered event handler.
 *
 *  RETURNS
 *      TRUE command is sent.
 *
 *----------------------------------------------------------------------------*/
extern bool TempSensorRead(void)
{
    bool status = FALSE;

    /* Return FALSE if already a read is in progress. */
    if (TIMER_INVALID == read_delay_tid)
    {
#ifdef TEMPERATURE_SENSOR_STTS751
        status = STTS751_InitiateOneShotRead();
#endif /* TEMPERATURE_SENSOR_STTS751 */
    }

    /* Command is issued without failure, start the delay timer. */
    if (status)
    {
#ifdef TEMPERATURE_SENSOR_STTS751
        read_delay_tid = TimerCreate(TEMP_READ_DELAY, TRUE,
                                     tempSensorReadyToRead);
#endif /* TEMPERATURE_SENSOR_STTS751 */
    }

    return status;
}


/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      tempsensor_hw.h
 *
 *  DESCRIPTION
 *      This file defines a interface to abstract hardware specifics of
 *      temperature sensor.
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __CSR_MESH_TEMPSENSOR_HW_H__
#define __CSR_MESH_TEMPSENSOR_HW_H__

#include "user_config.h"

/*============================================================================*
 *  Public Definitions
 *============================================================================*/
/* Converting 273.15 to 1/32kelvin results in 8740.8. Add the 0.025 to 
 * compensate loss due to integer division.
 */
#define INTEGER_DIV_LOSS_FACTOR (0.025)

/* Celsius to 1/32 kelvin conversion factor = (273.15 * 32) */
#define CELSIUS_TO_KELVIN_FACTOR \
                             (uint16)((273.15 + INTEGER_DIV_LOSS_FACTOR) * 32)

/* Number of Timers required for temperature sensor. */
#define NUM_TEMP_SENSOR_TIMERS      (1)

/* Function pointer for Temperature Sensor Event Callback. */
typedef void (*TEMPSENSOR_EVENT_HANDLER_T)(int16 temp);

/* Temperature Sensor Hardware Initialization function. */
extern bool TempSensorHardwareInit(TEMPSENSOR_EVENT_HANDLER_T handler);

/* This function initiates a Read temperature operation.
 * Temperature is reported in the Event Handler registered.
 */
extern bool TempSensorRead(void);

#if defined(ENABLE_TEMPERATURE_CONTROLLER) && !defined(DEBUG_ENABLE)
extern void HandlePIOEvent(uint32 pio_changed);
#endif /* defined(ENABLE_TEMPERATURE_CONTROLLER) && !defined(DEBUG_ENABLE) */

/* UART Receive callback */
#ifdef DEBUG_ENABLE
extern uint16 UartDataRxCallback ( void* p_data, uint16 data_count,
                                   uint16* p_num_additional_words );
#endif /* DEBUG_ENABLE */

#endif /* __CSR_MESH_TEMPSENSOR_HW_H__ */

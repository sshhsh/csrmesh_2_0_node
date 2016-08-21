/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      battery_hw.h
 *
 *  DESCRIPTION
 *      Header definitions for battery access routines
 *
 *****************************************************************************/

#ifndef __BATTERY_HW_H__
#define __BATTERY_HW_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>
#include <bt_event_types.h>

/*=============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Battery Level Full */
#define BATTERY_LEVEL_FULL                          (100)

/* Battery critical level, in percent */
#define BATTERY_CRITICAL_LEVEL                      (10)

/* Battery minimum and maximum voltages in mV */
#define BATTERY_FULL_BATTERY_VOLTAGE                (3000)          /* 3.0V */
#define BATTERY_FLAT_BATTERY_VOLTAGE                (1800)          /* 1.8V */

/* Battery state  */
#define BATTERY_STATE_POWERING_DEVICE               (0)
#define BATTERY_STATE_CHARGING                      (1)
#define BATTERY_STATE_EXTERNALLY_POWERED            (2)
#define BATTERY_STATE_SERVICE_REQUIRED              (3)
#define BATTERY_STATE_NEEDS_REPLACEMENT             (4)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function reads the battery level */
extern uint8 ReadBatteryLevel(void);

/* This function checks if the current battery voltage level has dropped below 
 * low threshold voltage
 */
extern bool CheckLowBatteryVoltage(void);

/* This function returns the battery state */
extern uint8 GetBatteryState(void);

/* This function sends the low battery indication onto the network */
extern void SendLowBatteryIndication(void);

#endif /* __BATTERY_HW_H__ */


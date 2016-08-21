/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *    battery_hw.c
 *
 * DESCRIPTION
 *    This file defines routines for reading current Battery level
 *
 ******************************************************************************/

/*=============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <gatt.h>
#include <battery.h>

/*=============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "battery_hw.h"
#include "battery_server.h"
#include "csr_mesh_switch.h"

/*=============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*-----------------------------------------------------------------------------*
 *  NAME
 *      ReadBatteryLevel
 *
 *  DESCRIPTION
 *      This function reads the battery level 
 *
 *  RETURNS
 *      Battery Level in percent
 *
 *----------------------------------------------------------------------------*/
extern uint8 ReadBatteryLevel(void)
{
    uint32 bat_voltage;
    uint32 bat_level;

    /* Read battery voltage and level it with minimum voltage */
    bat_voltage = BatteryReadVoltage();

    /* Level the read battery voltage to the minimum value */
    if(bat_voltage < BATTERY_FLAT_BATTERY_VOLTAGE)
    {
        bat_voltage = BATTERY_FLAT_BATTERY_VOLTAGE;
    }

    bat_voltage -= BATTERY_FLAT_BATTERY_VOLTAGE;

    /* Get battery level in percent */
    bat_level = (bat_voltage * 100) / (BATTERY_FULL_BATTERY_VOLTAGE - 
                                                  BATTERY_FLAT_BATTERY_VOLTAGE);

    /* Check the precision errors */
    if(bat_level > 100)
    {
        bat_level = 100;
    }

    return (uint8)bat_level;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      CheckLowBatteryVoltage
 *
 *  DESCRIPTION
 *      This function checks if the current battery volatage level has dropped 
 *      below low threshold voltage.
 *
 *  RETURNS
 *      Boolean-True or False
 *
 *---------------------------------------------------------------------------*/
extern bool CheckLowBatteryVoltage(void)
{
    bool low_battery = FALSE;

    if(BatteryReadVoltage() <= BatteryReadLowThreshold())
    {
        low_battery = TRUE;
        /* Broadcast battery state if it reduces below the threshold value */
        SendLowBatteryIndication();
    }
    return low_battery;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GetBatteryState
 *
 *  DESCRIPTION
 *      This function returns the battery state based on the present voltage 
 *      and the threshold voltage set.
 *
 *  RETURNS
 *      State of the battery
 *
 *---------------------------------------------------------------------------*/
extern uint8 GetBatteryState(void)
{
    uint8 battery_state = BATTERY_STATE_POWERING_DEVICE;

    if(BatteryReadVoltage() <= BatteryReadLowThreshold())
    {
        battery_state = BATTERY_STATE_NEEDS_REPLACEMENT;
    }
    return battery_state;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      SendLowBatteryIndication
 *
 *  DESCRIPTION
 *      This function sends the state message with the low battery indication
 *      onto the CSRmesh network.
 *
 *  RETURNS
 *      None
 *
 *---------------------------------------------------------------------------*/
extern void SendLowBatteryIndication(void)
{
    CSRMESH_BATTERY_STATE_T batt_state;

    batt_state.batterylevel = ReadBatteryLevel();
    batt_state.batterystate = BATTERY_STATE_NEEDS_REPLACEMENT;
    batt_state.tid = 0;

    /* Broadcast battery state if it reduces below the threshold value */
    BatteryState(DEFAULT_NW_ID, MESH_BROADCAST_ID, &batt_state);
}

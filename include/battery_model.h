/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  CSRmesh is a product of Qualcomm Technologies International Ltd.
 */
/*! \file Battery_model.h
 *
 *  \brief Defines CSRmesh Battery Model specific data structures\n
 */
/******************************************************************************/

#ifndef __BATTERY_MODEL_H__
#define __BATTERY_MODEL_H__

#include "csr_mesh_model_common.h"

/*! \addtogroup Battery_Model 
 *  \brief Battery Model API
 *
 * <br>The Battery Model reports the current battery state of a device that is powered by a battery.<br><br><h4>Battery State</h4>The battery model has the following states:<br><ul style='list-style:square;list-style-position: inside;'> <li style='list-style-type:square;'>BatteryLevel</li><li style='list-style-type:square;'>BatteryState</li></ul> <br><h5> BatteryLevel</h5>BatteryLevel is an unsigned 8-bit value as a percentage. The values 0x00 to 0x64 are valid, and all other values are reserved for future use. The value 0x00 represents a battery that has no energy capacity. The value 0x64 represents a battery with full capacity. Values between these two extremes represent a linear scaling from empty to full capacity.<br><br> <h5>BatteryState</h5>BatteryState is an 8-bit bit field. Each bit is a Boolean value that determines if a given state is true or false. The value of zero is false, and the value of one is true.<br><br> The following bits are defined:<br><ul style='list-style:square;list-style-position: inside;'> <li style='list-style-type:square;'>Battery is powering device=0</li><li style='list-style-type:square;'>Battery is charging=1</li><li style='list-style-type:square;'>Device is externally powered=2</li> <li style='list-style-type:square;'>Service is required for battery=3</li> <li style='list-style-type:square;'>Battery needs replacement=4</li></ul>All other bits are reserved.<br><br> <h4>Battery Periodic Behaviour</h4>The device transmits a new BATTERY_STATE message with the current state information when the BatteryLevel changes significantly or the BatteryState changes. The TID field can be set to any value. <br><br> <h6>Note:</h6> The definition of changing significantly is left to the implementation. It may mean when the battery level changes by 1% or 10% or 50% or some other value.
 * @{
 */

/*============================================================================*
    Public Definitions
 *============================================================================*/

/*! \brief CSRmesh Battery Model message types */

/*! \brief Getting Battery State: Upon receiving a BATTERY_GET_STATE message, the device responds with a BATTERY_STATE message with the current state information. */
typedef struct
{
    CsrUint8 tid; /*!< \brief Transaction identifier. The TID, or transaction identifier field is an 8-bit integer that identifies a given message with a known transaction from a source device. */
} CSRMESH_BATTERY_GET_STATE_T;

/*! \brief Current battery state */
typedef struct
{
    CsrUint8 batterylevel; /*!< \brief Battery level */
    CsrUint8 batterystate; /*!< \brief State of the battery */
    CsrUint8 tid; /*!< \brief Transaction identifier. The TID, or transaction identifier field is an 8-bit integer that identifies a given message with a known transaction from a source device. */
} CSRMESH_BATTERY_STATE_T;


/*!@} */
#endif /* __BATTERY_MODEL_H__ */


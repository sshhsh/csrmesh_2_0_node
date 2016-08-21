/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  CSRmesh is a product of Qualcomm Technologies International Ltd.
 */
/*! \file Power_model.h
 *
 *  \brief Defines CSRmesh Power Model specific data structures\n
 */
/******************************************************************************/

#ifndef __POWER_MODEL_H__
#define __POWER_MODEL_H__

#include "csr_mesh_model_common.h"

/*! \addtogroup Power_Model 
 *  \brief Power Model API
 *
 * <br>The Power Model controls the turning on and off of power for a device. A device may have multiple power states: on, off, and optionally standby. In addition to these states, a device may also remember if it is on standby before it is turned on so that it returns to standby mode after it is toggled to other power states.<br><br> The device power can be commanded directly to a given state, or the power state can be toggled to an opposite state. This means when a device is on, it will be toggled to off; when a device is off, it will be toggled to on. If a device supports the standby state, then the device will be toggled from standby to on from standby, and will be toggled from on from standby to standby. <br><br> The power model can be used in an individual device and at a wall power socket. A wall power socket may have multiple outlets, each of which can be individually switchable. To allow each power socket to be individually switched, each socket is assigned a separate GroupID.<br><br><h4>Power State</h4>The power model has the following state:<ul style='list-style:square;list-style-position: inside;'><li style='list-style-type:square;'>PowerState</li></ul><br><h5>Power State</h5>PowerState is an enumerated value held in an 8-bit value. The following values are defined:<ul style='list-style:square;list-style-position: inside;'><li style='list-style-type:square;'>Off=0x00. The device is powered off.</li><li style='list-style-type:square;'>On=0x01. The device is powered on.</li><li style='list-style-type:square;'>Standby=0x02. The device appears to be powered off, but is in a standby state so that it can turn on quicker than if it is off.</li><li style='list-style-type:square;'>OnFromStandby=0x03. The device is powered on from the standby state.</li></ul><br><h4>Power Background Behaviour</h4>The device may be interacted with directly, for example somebody presses a physical power switch on a device. When the power state changes locally on a device, it may send a POWER_STATE_NO_ACK message with the current state information.
 * @{
 */

/*============================================================================*
    Public Definitions
 *============================================================================*/

/*! \brief CSRmesh Power Model message types */

/*! \brief Setting Power State: Upon receiving a POWER_SET_STATE_NO_ACK message, the device sets the PowerState state value to the PowerState field. It then responds with a POWER_STATE message with the current state information. */
typedef struct
{
    CsrUint8 state; /*!< \brief State to set */
    CsrUint8 tid; /*!< \brief Transaction identifier. The  TID, or transaction identifier field is an 8-bit integer that identifies a given message with a known transaction from a source device. */
} CSRMESH_POWER_SET_STATE_T;

/*! \brief Toggling Power State: Upon receiving a POWER_Toggle_STATE_NO_ACK message, the device sets the PowerState state value as defined: 1.If the current PowerState is 0x00, Off, then PowerState should be set to 0x01, On. 2.If the current PowerState is 0x01, On, then PowerState should be set to 0x00, Off. 3.If the current PowerState is 0x02, Standby, then PowerState should be set to 0x03, OnFromStandby. 4.If the current PowerState is 0x03, OnFromStandby, then PowerState should be set to 0x02, Standby. Then the device responds with a POWER_STATE message with the current state information. */
typedef struct
{
    CsrUint8 tid; /*!< \brief Transaction identifier. The TID, or transaction identifier field is an 8-bit integer that identifies a given message with a known transaction from a source device. */
} CSRMESH_POWER_TOGGLE_STATE_T;

/*! \brief Getting Power State: Upon receiving a POWER_GET_STATE message, the device responds with a POWER_STATE message with the current state information. */
typedef struct
{
    CsrUint8 tid; /*!< \brief Transaction identifier. The TID, or transaction identifier field is an 8-bit integer that identifies a given message with a known transaction from a source device. */
} CSRMESH_POWER_GET_STATE_T;

/*! \brief Current power state */
typedef struct
{
    CsrUint8 state; /*!< \brief Power state */
    CsrUint8 tid; /*!< \brief Transaction identifier. The TID, or transaction identifier field is an 8-bit integer that identifies a given message with a known transaction from a source device. */
} CSRMESH_POWER_STATE_T;


/*!@} */
#endif /* __POWER_MODEL_H__ */


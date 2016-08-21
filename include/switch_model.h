/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  CSRmesh is a product of Qualcomm Technologies International Ltd.
 */
/*! \file Switch_model.h
 *
 *  \brief Defines CSRmesh Switch Model specific data structures\n
 */
/******************************************************************************/

#ifndef __SWITCH_MODEL_H__
#define __SWITCH_MODEL_H__

#include "csr_mesh_model_common.h"

/*! \addtogroup Switch_Model 
 *  \brief Switch Model API
 *
 * <br>The switch model does not define any messages. The switch model is configured using the group model. <br> Switch Client Behaviour: If the user presses the on switch with a given switch_number, then the client sends a POWER_SET_STATE message to the appropriate GroupID as identified by the Instance value from the group model. If the user rotates a dimmer switch, then the client may send a LIGHT_LEVEL_NO_ACK message to the appropriate GroupID as configured in the group model. If the user finishes rotating a dimmer switch, then the client may send a LIGHT_LEVEL, and await an acknowledgement from the lights that this dimmer controls.
 * @{
 */

/*============================================================================*
    Public Definitions
 *============================================================================*/

/*! \brief CSRmesh Switch Model message types */


/*!@} */
#endif /* __SWITCH_MODEL_H__ */


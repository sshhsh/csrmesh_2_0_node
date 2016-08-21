/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  CSRmesh is a product of Qualcomm Technologies International Ltd.
 */
/*! \file Actuator_client.h
 * 
 *  \brief This files provides the prototypes of the client functions defined
 *         in the CSRmesh Actuator model 
 */
/******************************************************************************/

#ifndef __ACTUATOR_CLIENT_H__
#define __ACTUATOR_CLIENT_H__

/*============================================================================*
 *  Local Header Files
 *===========================================================================*/

/*! \addtogroup Actuator_Client
 * @{
 */
#include "actuator_model.h"

/*============================================================================*
Public Function Prototypes
*============================================================================*/

/*----------------------------------------------------------------------------*
 * ActuatorModelClientInit
 */
/*! \brief 
 *      Initialises Actuator Model Client functionality.
 *  \param app_callback Pointer to the application callback function that will
 *                      be called when the model client receives a message.
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult ActuatorModelClientInit(CSRMESH_MODEL_CALLBACK_T app_callback);


/*----------------------------------------------------------------------------*
 * ActuatorGetTypes
 */
/*! \brief Upon receiving an ACTUATOR_GET_TYPES message, the device responds with an ACTUATOR_TYPES message with a list of supported types greater than or equal to the FirstType field. If the device does not support any types greater than or equal to FirstType, it sends an ACTUATOR_TYPES message with a zero length Types field.
 * 
 * This function packs the given parameters into a CSRmesh message and sends
 * it over the network. 
 * When a response is received the application  callback function is called 
 * to notify the \ref CSRMESH_ACTUATOR_TYPES 
 *
 * \param nw_id Network identifier over which the message has to be sent.
 * \param dest_id 16-bit identifier of the destination device/group
 * \param p_params Pointer to the message parameters of type
              \ref CSRMESH_ACTUATOR_GET_TYPES_T
 *
 * \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult ActuatorGetTypes(
                                  CsrUint8 nw_id,
                                  CsrUint16 dest_id,
                                  CSRMESH_ACTUATOR_GET_TYPES_T *p_params );

/*----------------------------------------------------------------------------*
 * ActuatorSetValue
 */
/*! \brief Get sensor state. Upon receiving an ACTUATOR_SET_VALUE_NO_ACK message, where the destination address is the device ID of this device and the Type field is a supported actuator type, the device shall immediately use the Value field for the given Type field. The meaning of this actuator value is not defined in this specification. Upon receiving an ACTUATOR_SET_VALUE_NO_ACK message, where the destination address is is the device ID of this device but the Type field is not a supported actuator type, the device shall ignore the message.
 * 
 * This function packs the given parameters into a CSRmesh message and sends
 * it over the network. 
 * When a response is received the application  callback function is called 
 * to notify the \ref CSRMESH_ACTUATOR_VALUE_ACK 
 *
 * \param nw_id Network identifier over which the message has to be sent.
 * \param dest_id 16-bit identifier of the destination device/group
 * \param p_params Pointer to the message parameters of type
              \ref CSRMESH_ACTUATOR_SET_VALUE_T
 * \param request_ack TRUE if an acknowledgement is required
 *
 * \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult ActuatorSetValue(
                                  CsrUint8 nw_id,
                                  CsrUint16 dest_id,
                                  CSRMESH_ACTUATOR_SET_VALUE_T *p_params,
                                  bool request_ack );

/*!@} */
#endif /* __ACTUATOR_CLIENT_H__ */


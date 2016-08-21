/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  CSRmesh is a product of Qualcomm Technologies International Ltd.
 */
/*! \file sensor_server.h
 *
 *  \brief This file provides the prototypes of the server functions defined
 *         in the CSRmesh Sensor model
 */
/******************************************************************************/

#ifndef __SENSOR_SERVER_H__
#define __SENSOR_SERVER_H__

/*! \addtogroup Sensor_Server
 * @{
 */
#include "sensor_model.h"

/*============================================================================*
    Public Function Definitions
 *============================================================================*/

/*----------------------------------------------------------------------------*
 * SensorModelInit
 */
/*! \brief Model Initialisation
 *  
 *   Registers the model handler with the CSRmesh. Sets the CSRmesh to report
 *   num_groups as the maximum number of groups supported for the model
 *
 *  \param nw_id Identifier of the network to which the model has to be
                 registered.
 *  \param group_id_list Pointer to a uint16 array to hold assigned group_ids. 
 *                       This must be NULL if no groups are supported
 *  \param num_groups Size of the group_id_list. This must be 0 if no groups
 *                    are supported.
 *  \param app_callback Pointer to the application callback function. This
 *                    function will be called to notify all model specific messages
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult SensorModelInit(CsrUint8 nw_id, CsrUint16 *group_id_list, CsrUint16 num_groups,
                                         CSRMESH_MODEL_CALLBACK_T app_callback);


/*----------------------------------------------------------------------------*
 * SensorTypes
 */
/*! \brief Sensor types
 * 
 * This function packs the given parameters into a CSRmesh message and sends
 * it over the network. 
 * 
 *
 * \param nw_id Network identifier over which the message has to be sent.
 * \param dest_id 16-bit identifier of the destination device/group
 * \param p_params Pointer to the message parameters of type
              \ref CSRMESH_SENSOR_TYPES_T
 *
 * \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult SensorTypes(
                                  CsrUint8 nw_id,
                                  CsrUint16 dest_id,
                                  CSRMESH_SENSOR_TYPES_T *p_params);

/*----------------------------------------------------------------------------*
 * SensorState
 */
/*! \brief Current sensor state
 * 
 * This function packs the given parameters into a CSRmesh message and sends
 * it over the network. 
 * 
 *
 * \param nw_id Network identifier over which the message has to be sent.
 * \param dest_id 16-bit identifier of the destination device/group
 * \param p_params Pointer to the message parameters of type
              \ref CSRMESH_SENSOR_STATE_T
 *
 * \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult SensorState(
                                  CsrUint8 nw_id,
                                  CsrUint16 dest_id,
                                  CSRMESH_SENSOR_STATE_T *p_params);

/*----------------------------------------------------------------------------*
 * SensorValue
 */
/*! \brief Current sensor value
 * 
 * This function packs the given parameters into a CSRmesh message and sends
 * it over the network. 
 * 
 *
 * \param nw_id Network identifier over which the message has to be sent.
 * \param dest_id 16-bit identifier of the destination device/group
 * \param p_params Pointer to the message parameters of type
              \ref CSRMESH_SENSOR_VALUE_T
 *
 * \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult SensorValue(
                                  CsrUint8 nw_id,
                                  CsrUint16 dest_id,
                                  CSRMESH_SENSOR_VALUE_T *p_params);

/*----------------------------------------------------------------------------*
 * SensorMissing
 */
/*! \brief Sensor data is missing. Proxy Behaviour: Upon receiving a SENSOR_MISSING message, the proxy determines if it has remembered this type and value and then writes that type and value to the device that sent the message.
 * 
 * This function packs the given parameters into a CSRmesh message and sends
 * it over the network. 
 * 
 *
 * \param nw_id Network identifier over which the message has to be sent.
 * \param dest_id 16-bit identifier of the destination device/group
 * \param p_params Pointer to the message parameters of type
              \ref CSRMESH_SENSOR_MISSING_T
 *
 * \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult SensorMissing(
                                  CsrUint8 nw_id,
                                  CsrUint16 dest_id,
                                  CSRMESH_SENSOR_MISSING_T *p_params);

/*!@} */
#endif /* __SENSOR_SERVER_H__ */


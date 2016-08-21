/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  CSRmesh is a product of Qualcomm Technologies International Ltd.
 */
/*! \file Watchdog_client.h
 * 
 *  \brief This files provides the prototypes of the client functions defined
 *         in the CSRmesh Watchdog model 
 */
/******************************************************************************/

#ifndef __WATCHDOG_CLIENT_H__
#define __WATCHDOG_CLIENT_H__

/*============================================================================*
 *  Local Header Files
 *===========================================================================*/

/*! \addtogroup Watchdog_Client
 * @{
 */
#include "watchdog_model.h"

/*============================================================================*
Public Function Prototypes
*============================================================================*/

/*----------------------------------------------------------------------------*
 * WatchdogModelClientInit
 */
/*! \brief 
 *      Initialises Watchdog Model Client functionality.
 *  \param app_callback Pointer to the application callback function that will
 *                      be called when the model client receives a message.
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult WatchdogModelClientInit(CSRMESH_MODEL_CALLBACK_T app_callback);


/*----------------------------------------------------------------------------*
 * WatchdogMessage
 */
/*! \brief Upon receiving a WATCHDOG_MESSAGE message, if the RspSize field is set to a non-zero value, then the device shall respond with a WATCHDOG_MESSAGE with the RspSize field set to zero, and RspSize -1 octets of additional RandomData.
 * 
 * This function packs the given parameters into a CSRmesh message and sends
 * it over the network. 
 * When a response is received the application  callback function is called 
 * to notify the \ref CSRMESH_WATCHDOG_MESSAGE 
 *
 * \param nw_id Network identifier over which the message has to be sent.
 * \param dest_id 16-bit identifier of the destination device/group
 * \param p_params Pointer to the message parameters of type
              \ref CSRMESH_WATCHDOG_MESSAGE_T
 *
 * \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult WatchdogMessage(
                                  CsrUint8 nw_id,
                                  CsrUint16 dest_id,
                                  CSRMESH_WATCHDOG_MESSAGE_T *p_params );

/*----------------------------------------------------------------------------*
 * WatchdogSetInterval
 */
/*! \brief Upon receiving a WATCHDOG_SET_INTERVAL message, the device shall save the Interval and ActiveAfterTime fields into the Interval and ActiveAfterTime variables and respond with a WATCHDOG_INTERVAL message with the current variable values.
 * 
 * This function packs the given parameters into a CSRmesh message and sends
 * it over the network. 
 * When a response is received the application  callback function is called 
 * to notify the \ref CSRMESH_WATCHDOG_INTERVAL 
 *
 * \param nw_id Network identifier over which the message has to be sent.
 * \param dest_id 16-bit identifier of the destination device/group
 * \param p_params Pointer to the message parameters of type
              \ref CSRMESH_WATCHDOG_SET_INTERVAL_T
 *
 * \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
extern CSRmeshResult WatchdogSetInterval(
                                  CsrUint8 nw_id,
                                  CsrUint16 dest_id,
                                  CSRMESH_WATCHDOG_SET_INTERVAL_T *p_params );

/*!@} */
#endif /* __WATCHDOG_CLIENT_H__ */


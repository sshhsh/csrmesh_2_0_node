 /******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *****************************************************************************/
/*! \file csr_sched.h
 *  \brief CSRmesh library LE bearer scheduling configuration and control functions
 *
 *   This file contains the functions to provide the application to
 *   configuration and control of LE Bearer.
 *
 */
 /******************************************************************************/

#ifndef __CSR_SCHED_H__
#define __CSR_SCHED_H__

#include "csr_sched_types.h"

/*! \addtogroup CSRmesh
 * @{
 */

#if defined(CSR101x) || defined(CSR101x_A05)
/*-----------------------------------------------------------------------------*
 * "CSRmesh Bearer-Scheduling Wrapper APIs
 *-----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 * CSRSchedSetConfigParams
 */
/*! \brief  Configure Mesh and Generic LE parameters
 *
 *  Applicaton uses this API to Configure MESH and Generic LE parameters
 *
 *  \param le_params  This structure contains and Mesh or Non-mesh 
 *  scheduling parameters.
 *
 *  \returns CSRSchedResult. Refer to \ref CSRSchedResult.
 */
/*---------------------------------------------------------------------------*/
CSRSchedResult CSRSchedSetConfigParams(CSR_SCHED_LE_PARAMS_T *le_params);

/*----------------------------------------------------------------------------*
 * CSRSchedSendUserAdv
 */
/*! \brief Transmit Non-Connectable/Connectable Application data. 
 *
 * Application Uses this API to transmit user
 * advertising data(CONNECTABLE/NON-CONNECTABLE)
 *
 *  \param le_adv_data  contains advertising paramters
 *  advertising data and scan response data.
 *
 *  \paramcallBack  callback function registered by the application.
 *  Scheduler uses this callback to notify the application regarding the 
 *  successful transmission of user adv packet.
 *
 *  \returns CSRSchedResult. Refer to \ref CSRSchedResult.
 */
/*---------------------------------------------------------------------------*/
CSRSchedResult CSRSchedSendUserAdv(CSR_SCHED_ADV_DATA_T *le_adv_data, 
                                   CSR_SCHED_USER_ADV_NOTIFY_CB_T callBack);

/*----------------------------------------------------------------------------*
 * CSRSchedGetConfigParams
 */
/*! \brief Get scheduler configuration parameters
 *
 *  Application uses this API to Get the current configuration .
 *
 *  \param le_params  This structure contains Generic LE and Mesh LE
 *  parameters.
 *
 *  \returns CSRSchedResult. Refer to \ref CSRSchedResult.
 */
/*---------------------------------------------------------------------------*/
CSRSchedResult CSRSchedGetConfigParams (CSR_SCHED_LE_PARAMS_T *le_params);

/*----------------------------------------------------------------------------*
 * CSRSchedHandleIncomingData
 */
/*! \brief Forward Mesh messages to the scheduler
 *
 *  This API should get called during the reception of incoming mesh message
 *
 *  \param data_event Specifies the bearer type over which the mesh data is received.
 *
 *  \param data  Mesh data received.
 *
 *  \param length  Length of the mesh data.
 *.
 *  \param rssi  Received signal strength.
 *
 *  \returns CSRSchedResult. Refer to \ref CSRSchedResult.
 */
/*---------------------------------------------------------------------------*/
CSRmeshResult CSRSchedHandleIncomingData(CSR_SCHED_INCOMING_DATA_EVENT_T data_event, 
                                         CsrUint8* data, 
                                         CsrUint8 length, 
                                         CsrInt8 rssi);
/*----------------------------------------------------------------------------*
 * CSRSchedEnableListening
 */
/*! \brief Starts or stops LE scan operation
 *
 *  Application uses this api to start/stop LE scan operation.
 *
 *  \param enable TRUE / FALSE
 *
 *  \returns CSRSchedResult. Refer to \ref CSRSchedResult.
 */
/*---------------------------------------------------------------------------*/
CSRSchedResult CSRSchedEnableListening(CsrBool enable);

/*----------------------------------------------------------------------------*
 * IsCSRSchedRunning
 */
/*! \brief Indicates about the scheduler state
 *
 *  Application uses this api to check whether scheduler is ready for sending
 *  advtertisements.
 *
 *  \param enable TRUE / FALSE
 *
 *  \returns CSRSchedResult. Refer to \ref CSRSchedResult.
 */
/*---------------------------------------------------------------------------*/
CsrBool IsCSRSchedRunning(void);

/*----------------------------------------------------------------------------*
 * CSRSchedStart
 */
/*! \brief Start scheduling of LE scan and advertisement
 *
 *  This API starts scheduling of LE scan and advertisement 
 *
 *  \returns CSRSchedResult. Refer to \ref CSRSchedResult.
 */
/*---------------------------------------------------------------------------*/
CSRSchedResult CSRSchedStart(void);

/*----------------------------------------------------------------------------*
 * CSRSchedNotifyGattEvent
 */
/*! \brief Initialize CSRmesh core mesh stack
 *
 *  Application uses this API to notify the scheduler regarding 
 *  the GATT connection state event
 *
 *  \param gatt_event_type  GATT event type 
 *
 *  \param gatt_event_data  GATT event data
 *
 *  \param call_back  callback function to notify mesh data to client
 *
 *  \returns Nothing
 */
/*---------------------------------------------------------------------------*/
void CSRSchedNotifyGattEvent(CSR_SCHED_GATT_EVENT_T gatt_event_type,
                             CSR_SCHED_GATT_EVENT_DATA_T *gatt_event_data,
                             CSR_SCHED_NOTIFY_GATT_CB_T call_back);


#endif /* defined(CSR101x) || defined(CSR101x_A05) */

/*!@} */

#endif /* __CSR_SCHED_H__*/


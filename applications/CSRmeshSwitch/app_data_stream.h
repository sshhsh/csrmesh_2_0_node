/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      app_data_stream.h
 *
 *  DESCRIPTION
 *      Header definitions for application data stream implementation
 *
 *****************************************************************************/

#ifndef __APP_DATA_STREAM_H__
#define __APP_DATA_STREAM_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>
#include <bt_event_types.h>

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include "user_config.h"

/*============================================================================*
 *  CSRmesh Header Files
 *============================================================================*/
#include <csr_mesh.h>
#include <data_server.h>
#include <data_client.h>


/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Application protocol codes used to exchange device data over the
 * data stream model
 */
typedef enum
{
    CSR_DEVICE_INFO_REQ = 0x01,
    CSR_DEVICE_INFO_RSP = 0x02,
    CSR_DEVICE_INFO_SET = 0x03,
    CSR_DEVICE_INFO_RESET = 0x04
}APP_DATA_STREAM_CODE_T;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
void AppDataStreamInit(uint16 *data_model_groups, uint16 num_groups);

CSRmeshResult AppDataClientHandler(CSRMESH_MODEL_EVENT_T event_code,
                                   CSRMESH_EVENT_DATA_T* data,
                                   CsrUint16 length,
                                   void **state_data);
CSRmeshResult AppDataServerHandler(CSRMESH_MODEL_EVENT_T event_code,
                                   CSRMESH_EVENT_DATA_T* data,
                                   CsrUint16 length,
                                   void **state_data);
#endif /* __APP_DATA_STREAM_H__ */


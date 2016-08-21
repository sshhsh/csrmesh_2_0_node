/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      app_event_handler.h
 *
 *
 ******************************************************************************/
#ifndef __APP_MESH_EVENT_HANDLER_H__
#define __APP_MESH_EVENT_HANDLER_H__

#include <csr_types.h>
#include <csr_mesh_types.h>

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

extern void CSRmeshAppProcessMeshEvent(
                                CSR_MESH_APP_EVENT_DATA_T eventDataCallback);
CSRmeshResult AppLightClientHandler(CSRMESH_MODEL_EVENT_T event_code, 
                                    CSRMESH_EVENT_DATA_T* data,
                                    CsrUint16 length, 
                                    void **state_data);
CSRmeshResult AppPowerClientHandler(CSRMESH_MODEL_EVENT_T event_code,
                                    CSRMESH_EVENT_DATA_T* data,
                                    CsrUint16 length,
                                    void **state_data);
CSRmeshResult AppAttentionEventHandler(CSRMESH_MODEL_EVENT_T event_code,
                                       CSRMESH_EVENT_DATA_T* data,
                                       CsrUint16 length,
                                       void **state_data);
CSRmeshResult AppBatteryEventHandler(CSRMESH_MODEL_EVENT_T event_code,
                                     CSRMESH_EVENT_DATA_T* data,
                                     CsrUint16 length,
                                     void **state_data);

extern void EnableHighDutyScanMode(bool enable);
#endif /* __APP_MESH_EVENT_HANDLER_H__ */

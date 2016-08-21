/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      app_mesh_event_handler.h
 *
 *
 ******************************************************************************/
#ifndef __APP_MESH_EVENT_HANDLER_H__
#define __APP_MESH_EVENT_HANDLER_H__

#include <csr_types.h>
#include <csr_mesh_types.h>

/* Number of Supported Sensors. */
#define NUM_SENSORS_SUPPORTED               (2)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

extern void CSRmeshAppProcessMeshEvent(
                CSR_MESH_APP_EVENT_DATA_T eventDataCallback);

CSRmeshResult AppSensorEventHandler(CSRMESH_MODEL_EVENT_T event_code, 
                                    CSRMESH_EVENT_DATA_T* data,
                                    CsrUint16 length, 
                                    void **state_data);
CSRmeshResult AppActuatorEventHandler(CSRMESH_MODEL_EVENT_T event_code, 
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

extern void InitiliseSensorData(void);
extern void EnableTempReading(void);
extern void ReadSensorDataFromNVM(uint16 idx);
extern bool IsSensorConfigured(void);
extern void WriteSensorDataToNVM(uint16 idx);
extern void ConfigureSensor(bool old_config);
extern void EnableHighDutyScanMode(bool enable);
extern void StartTempTransmission(void);
#endif /* __APP_MESH_EVENT_HANDLER_H__ */

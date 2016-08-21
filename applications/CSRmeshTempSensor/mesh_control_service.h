/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      mesh_control_service.h
 *
 *  DESCRIPTION
 *      Header definitions for mesh control service
 *
 *****************************************************************************/

#ifndef __MESH_CONTROL_SERVICE_H__
#define __MESH_CONTROL_SERVICE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>
#include <bt_event_types.h>

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

#define MESH_LONGEST_MSG_LEN                              (27)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
/* This function is used to initialise Mesh Control Service data structure.*/
extern void MeshControlServiceDataInit(void);

/* This function handles read operation on the Mesh Control service
 * attributes maintained by the application
 */
extern void MeshControlHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* This function handles write operations on the Mesh Control service
 * attributes maintained by the application
 */
extern void MeshControlHandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* This function is used to check if the handle belongs to the Mesh Control
 * service
 */
extern bool MeshControlCheckHandleRange(uint16 handle);

/* This function notifies responses received on the mesh to the Heater 
 * client
 */
extern void MeshControlNotifyResponse(uint16 ucid, uint8 *mtl_msg, uint8 length);


#endif /* __MESH_CONTROL_SERVICE_H__ */


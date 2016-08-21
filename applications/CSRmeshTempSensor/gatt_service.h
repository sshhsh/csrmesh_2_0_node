/*******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      gatt_service.h
 *
 *  DESCRIPTION
 *      Header definitions for GATT service
 *
 *****************************************************************************/

#ifndef __GATT_SERVICE_H__
#define __GATT_SERVICE_H__

/*=============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "user_config.h"

/*=============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <bt_event_types.h>

/*=============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function is used to initialise gatt service data structure.*/
extern void GattDataInit(void);

/* This function allows other modules to read whether the bonded device
 * has requested indications on the Service Changed characteristic. */
extern bool GattServiceChangedIndActive(void);

/* This function handles READ operations on GATT service attributes. */
extern void GattHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* This function handles READ operations on GATT service attributes. */
extern void GattHandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* Determine whether a given handle is one handled by the GAP module. */
extern bool GattCheckHandleRange(uint16 handle);

#endif /* __GATT_SERVICE_H__ */

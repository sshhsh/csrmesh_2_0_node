/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      gap_service.h
 *
 *  DESCRIPTION
 *      Header definitions for GAP service
 *
 *****************************************************************************/

#ifndef __GAP_SERVICE_H__
#define __GAP_SERVICE_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>
#include <bt_event_types.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "gap_conn_params.h"

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function is used to initialise GAP service data structure.*/
extern void GapDataInit(void);

/* This function handles read operation on GAP service attributes
 * maintained by the application
 */
extern void GapHandleAccessRead(GATT_ACCESS_IND_T *p_ind);

/* This function handles write operation on GAP service attributes
 * maintained by the application
 */
extern void GapHandleAccessWrite(GATT_ACCESS_IND_T *p_ind);

/* This function is used to read GAP specific data stored in NVM */
extern void GapReadDataFromNVM(uint16 *p_offset);

/* This function is used to write GAP specific data to NVM for 
 * the first time during application initialisation
 */
extern void GapInitWriteDataToNVM(uint16 *p_offset);

/* This function is used to check if the handle belongs to the GAP 
 * service
 */
extern bool GapCheckHandleRange(uint16 handle);

/* This function is used to get the reference to the 'g_device_name' 
 * array, which contains AD Type and device name
 */
extern uint8 *GapGetNameAndLength(uint16 *p_name_length);

#ifdef NVM_TYPE_FLASH
/* This function writes the GAP service data in NVM */
extern void WriteGapServiceDataInNVM(void);
#endif /* NVM_TYPE_FLASH */

#endif /* __GAP_SERVICE_H__ */

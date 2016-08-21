/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      gap_uuids.h
 *
 *  DESCRIPTION
 *      UUID MACROs for GAP service
 *
 *****************************************************************************/

#ifndef __GAP_UUIDS_H__
#define __GAP_UUIDS_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Brackets should not be used around the value of a macro. The parser 
 * which creates .c and .h files from .db file doesn't understand  brackets 
 * and will raise syntax errors.
 */

/* For UUID values, refer http://developer.bluetooth.org/gatt/services/
 * Pages/ServiceViewer.aspx?u=org.bluetooth.service.generic_access.xml
 */

#define UUID_GAP                                       0x1800

#define UUID_DEVICE_NAME                               0x2A00

#define UUID_APPEARANCE                                0x2A01

#define UUID_PER_PREF_CONN_PARAMS                      0x2A04

#endif /* __GAP_UUIDS_H__ */

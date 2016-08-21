/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      mesh_control_service_uuids.h
 *
 *  DESCRIPTION
 *      UUID MACROs for the CSRmesh Service.
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __MESH_CONTROL_SERIVCE_UUIDS_H__
#define __MESH_CONTROL_SERIVCE_UUIDS_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* Brackets should not be used around the value of a macro. The parser which
 * creates .c and .h files from .db file doesn't understand brackets
 * and will raise syntax errors.
 */

/* The UUIDs given below are CSR specific UUIDS. */

/* Mesh Control service UUID */
#define MESH_CONTROL_SERVICE_UUID             0xFEF1

/* Network Key characteristic UUID */
#define NETWORK_KEY_UUID                      0xC4EDC0009DAF11E3800000025B000B00

/* Device ID characteristic UUID */
#define DEVICE_ID_UUID                        0xC4EDC0009DAF11E3800200025B000B00

/* Device UUID characteristic */
#define DEVICE_UUID_UUID                      0xC4EDC0009DAF11E3800100025B000B00

/* Mesh Transport Layer control point characteristic UUID */
#define MTL_CONTINUATION_CP_UUID              0xC4EDC0009DAF11E3800300025B000B00
#define MTL_COMPLETE_CP_UUID                  0xC4EDC0009DAF11E3800400025B000B00

/* Mesh Transport Layer Time To Live characteristic UUID */
#define MTL_TTL_UUID                          0xC4EDC0009DAF11E3800500025B000B00

/* Mesh Appearance characteristic UUID */
#define MESH_APPEARANCE_UUID                  0xC4EDC0009DAF11E3800600025B000B00

#endif /* __MESH_CONTROL_SERIVCE_UUIDS_H__ */


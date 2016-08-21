/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      csr_mesh_heater_util.h
 *
 *
 ******************************************************************************/
#ifndef __CSR_MESH_HEATER_UTIL_H__
#define __CSR_MESH_HEATER_UTIL_H__

#include <timer.h>
#include "csr_types.h"

/*============================================================================*
 *  Public Data
 *============================================================================*/
extern uint8 mesh_ad_data[];
/* Mesh appearance value */
extern CSR_MESH_DEVICE_APPEARANCE_T appearance;

/*============================================================================*
 *  Public Definitions
 *============================================================================*/
/* CSRmesh Heater appearance value*/
#define APPEARANCE_CSR_MESH_HEATER_VALUE       0x1063

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Reads the application NVM values from the NVM store */
extern void ReadPersistentStore(void);

/* This function updates the relay and promiscuous mode of the GATT and
 * and the LE Advert bearers
 */
extern void AppUpdateBearerState(CSR_MESH_TRANSMIT_STATE_T *p_bearer_state);

/* This function kick starts a timer to send periodic CSRmesh UUID Messages
 * and starts blinking Blue LED to visually indicate association ready status
 */
extern void InitiateAssociation(void);

/* This function handles the CSRmesh GROUP SET message to set Group IDs as of 
 * the supported models as assigned in the message
 */
extern bool HandleGroupSetMsg(CSR_MESH_GROUP_ID_RELATED_DATA_T msg);

#ifdef USE_STATIC_RANDOM_ADDRESS
/* Generates a 48 bit random bluetooth address */
extern  void GenerateStaticRandomAddress(BD_ADDR_T *addr);
#endif/* USE_STATIC_RANDOM_ADDRESS */

#endif /* __CSR_MESH_HEATER_UTIL_H__ */

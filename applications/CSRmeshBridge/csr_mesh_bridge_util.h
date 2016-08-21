/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      csr_mesh_bridge_util.h
 *
 *
 ******************************************************************************/
#ifndef __CSR_MESH_BRIDGE_UTIL_H__
#define __CSR_MESH_BRIDGE_UTIL_H__

#include <timer.h>
#include "csr_types.h"

extern uint8 mesh_ad_data[];

/*============================================================================*
 *  Public Definitions
 *============================================================================*/
/* CSRmesh bridge appearance value */
#define APPEARANCE_CSRMESH_BRIDGE_VALUE         0x1064

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* Reads the application NVM values from the NVM store */
extern void ReadPersistentStore(void);

/* This function updates the relay and promiscuous mode of the GATT and
 * and the LE Advert bearers
 */
extern void AppUpdateBearerState(CSR_MESH_TRANSMIT_STATE_T *p_bearer_state);

#ifdef USE_STATIC_RANDOM_ADDRESS
/* Generates a 48 bit random bluetooth address */
extern  void generateStaticRandomAddress(BD_ADDR_T *addr);
#endif

#endif /* __CSR_MESH_BRIDGE_UTIL_H__ */

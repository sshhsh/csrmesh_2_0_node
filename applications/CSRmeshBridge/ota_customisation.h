/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      ota_customisation.h
 *
 *  DESCRIPTION
 *      Customisation requirements for the CSR OTAU functionality.
 *
 *****************************************************************************/

#ifndef __OTA_CUSTOMISATION_H__
#define __OTA_CUSTOMISATION_H__

/*=============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "user_config.h"

/* ** CUSTOMISATION **
 * The following header file names may need altering to match your application.
 */

#include "app_gatt.h"
#include "app_gatt_db.h"
#include "csr_mesh_bridge.h"

/*=============================================================================*
 *  Private Definitions
 *============================================================================*/

/* ** CUSTOMISATION **
 * Change these definitions to match your application.
 */
#define CONNECTION_CID      g_bridgeapp_data.gatt_data.st_ucid
#define IS_PAIRED           g_bridgeapp_data.gatt_data.paired
#define CONN_CENTRAL_ADDR   g_bridgeapp_data.gatt_data.con_bd_addr
#define DEVICE_RANDOM_ADDR  g_bridgeapp_data.random_bd_addr

#endif /* __OTA_CUSTOMISATION_H__ */


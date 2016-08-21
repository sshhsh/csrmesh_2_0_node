 /******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd
 *****************************************************************************/
/*! \file csr_mesh_config.h
 *  \brief CSRmesh library configuration parameters
 *
 *   This file contains the library configuration parameters for UUID type,
 *   authorization code,Multi network support etc.
 *
 *   NOTE: This library includes the Mesh Transport Layer, Mesh Control
 *   Layer and Mesh Association Layer functionality.
 */
 /*****************************************************************************/
#ifndef __CSR_MESH_CONFIG_H__
#define __CSR_MESH_CONFIG_H__

#include "csr_types.h"

/*! \addtogroup CSRmesh
 * @{
 */

/*! \brief CSR Mesh Unit test realted macro 
*/
#define CSR_MESH_UT_ENABLE                           (0)

/*! \brief  CSR Mesh Seen Packet Cache Size 
*/
#define CSR_MESH_SEEN_PACKET_CACHE_SIZE              (32)

/*! \brief  CSR Mesh Macro to enable Random UUID 
*/ 
#define CSR_MESH_RANDOM_UUID_ENABLE                  (0)
/*! \brief CSR Mesh Macro to enable Panic 
*/ 
#define CSR_MESH_PANIC_ENABLE                        (1)

/*! \brief  CSR Mesh Macro to enable Config role related source code 
*/ 
#define CSR_MESH_SUPPORT_CONFIG_ROLE                 (0)

/*! \brief  CSR Mesh Macro to enable MAC calculation from saved context
*/ 
#define CSR_MESH_ENABLE_MAC_CALC_FROM_SAVED_CONTEXT  (1)

/*! \brief CSR Mesh Macro to enable Number of allowed networks 
*/ 
#define CSR_MESH_MAX_NO_OF_NWKS                      (1)

/*! \brief  CSR MeshMacro to support  MulitNetwork
*/ 
#define CSR_MESH_MULTI_NWK_ENABLE                    (0)

/*! \brief  CSR Mesh Macro to enable no of supported Bearer
*/ 
#define CSR_MESH_MAX_NO_OF_SUPPORTED_BEARER          (2)

/*! \brief CSR Mesh Macro to enable Bridging of mesh data from GATT to LE
*/
#define CSR_MESH_ENABLE_BRIDGE_ON_GATT               (0)

#if defined(CSR101x) || defined(CSR101x_A05)
/*! \brief  CSR Mesh Macro to enable On Chip 
*/ 
#define CSR_MESH_ON_CHIP                             (1)
#endif

#if ((CSR_MESH_MULTI_NWK_ENABLE != 0) && (CSR_MESH_MAX_NO_OF_NWKS <= 1))
#error "CSR_MESH_MAX_NO_OF_NWKS must be greater than 1 if multiple network is enabled"
#elif ((CSR_MESH_MULTI_NWK_ENABLE == 0) && (CSR_MESH_MAX_NO_OF_NWKS != 1))
#error "CSR_MESH_MAX_NO_OF_NWKS must be 1 if multiple network is disabled"
#endif


/*!@} */
#endif

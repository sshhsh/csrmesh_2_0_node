 /******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *****************************************************************************/
/*! \file csr_mesh_result.h
 *  \brief CSRmesh library result type
 *
 *   This file contains the different result types the application will receive
 *   as a result of invoking the Mesh APIs.
 *
 *   NOTE: This library includes the Mesh Transport Layer, Mesh Control
 *   Layer and Mesh Association Layer functionality.
 */
 /*****************************************************************************/

#ifndef __CSR_MESH_RESULT_H__
#define __CSR_MESH_RESULT_H__

#include "csr_types.h"

/*! \addtogroup CSRmesh
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

/*!\brief CSR Mesh operation status Type */ 
typedef CsrUint16 CSRmeshResult;


#define CSR_MESH_RESULT_SUCCESS                     ((CSRmeshResult) 0x0000) /*!<\briefCSR Mesh Operation status Success */ 
#define CSR_MESH_RESULT_INPROGRESS                  ((CSRmeshResult) 0x0001)/*!<\brief  CSR Mesh Operation status InProgress */ 
#define CSR_MESH_RESULT_MESH_INVALID_STATE          ((CSRmeshResult) 0x0002)/*!<\brief  CSR Mesh Operation status Mesh_Invalid_State */ 
#define CSR_MESH_RESULT_MODEL_NOT_REGISTERED        ((CSRmeshResult) 0x0003)/*!<\brief  CSR Mesh Operation statusModel_Not_Registered */ 
#define CSR_MESH_RESULT_MODEL_ALREADY_REGISTERD     ((CSRmeshResult) 0x0004)/*!<\brief  CSR Mesh Operation status Model_Already_Registered */ 
#define CSR_MESH_RESULT_ROLE_NOT_SUPPORTED          ((CSRmeshResult) 0x0005)/*!<\brief  CSR Mesh Operation status Role_Not_Supported */ 
#define CSR_MESH_RESULT_INVALID_NWK_ID              ((CSRmeshResult) 0x0006)/*!<\brief  CSR MeshOperation status Invalid_Network_id */ 
#define CSR_MESH_RESULT_EXCEED_MAX_NO_OF_NWKS       ((CSRmeshResult) 0x0007)/*!<\brief  CSR MeshOperation status Exceed_Max_No_Of_Network */ 
#define CSR_MESH_RESULT_NOT_READY                   ((CSRmeshResult) 0x0008)/*!<\brief  CSR Mesh Operation statusNot_ready */ 
#define CSR_MESH_RESULT_MASP_ALREADY_ASSOCIATING    ((CSRmeshResult) 0x0009)/*!<\brief  CSR Mesh Operation status Masp_Already_Associating */ 
#define CSR_MESH_RESULT_API_NOT_SUPPORTED           ((CSRmeshResult) 0x000A)/*!<\brief  CSR Mesh Operation status Api_Not_Supported */ 
#define CSR_MESH_RESULT_INVALID_ARG                 ((CSRmeshResult) 0x000B)/*!<\brief  CSR Mesh Operation status Invalid argument */ 
#define CSR_MESH_RESULT_FAILURE                     ((CSRmeshResult) 0xFFFF)/*!<\brief  CSR Mesh Operation status Failure */ 

#ifdef __cplusplus
}
#endif

/*!@} */
#endif

 /*****************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *****************************************************************************/
/*! \file csr_mesh.h
 *  \brief CSRmesh library configuration and control functions
 *
 *   This file contains the functions to provide the application with
 *   access to the CSRmesh library
 *
 *   NOTE: This library includes the Mesh Transport Layer, Mesh Control
 *   Layer and Mesh Association Layer functionality.
 */
 /*****************************************************************************/
 
#ifndef __CSR_MESH_H__
#define __CSR_MESH_H__

#include "csr_mesh_types.h"

/*! \addtogroup CSRmesh
 * @{
 */

/*============================================================================*
Public Function Implementations
*============================================================================*/

/**************************CSRmesh Generic APIs*******************************/

/*----------------------------------------------------------------------------*
 * CSRmeshInit
 */
/*! \brief Initialize CSRmesh core mesh stack
 *
 *  Initialize MASP, MCP & MTL Layers.
 *
 *  \param configFlag  The configuration flag identifies the role of 
 *  the device (configuring / non-configuring).
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*---------------------------------------------------------------------------*/
CSRmeshResult CSRmeshInit(CSR_MESH_CONFIG_FLAG_T configFlag);

/*----------------------------------------------------------------------------*
 * CSRmeshRegisterAppCallback
 */
/*! \brief Register application callback
 *
 *  This API registers an application call-back to the stack. All the stack 
 *  events are notified to this registerd call-back. For supporting mutiple 
 *  applications a seperate multiplexer module is required. The multiplexer 
 *  module should take care of notifying the stack event to all the interested 
 *  applications.
 *
 *  \param callback application call-back to register
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*---------------------------------------------------------------------------*/
CSRmeshResult CSRmeshRegisterAppCallback(CSR_MESH_APP_CB_T callback);

/*----------------------------------------------------------------------------*
 * CSRmeshStart
 */
/*! \brief Start the CSRmesh system
 *
 *  Initializes Bearer Layer Successful completion is indication that stack 
 *  is ready for Rx/Tx.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*---------------------------------------------------------------------------*/
CSRmeshResult CSRmeshStart(void);

/*----------------------------------------------------------------------------*
 * CSRmeshStop
 */
/*! \brief Stop the CSRmesh system
 *
 *  Stops the Rx/Tx on CSRmesh.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*---------------------------------------------------------------------------*/
CSRmeshResult CSRmeshStop(void);

/*----------------------------------------------------------------------------*
 * CSRmeshReset
 */
/*! \brief Reset the CSRmesh library
 *
 *  Resets the CSRmesh library and clears all data relevant to each layer.
 *  The network association info is retained which can be removed only using
 *  the config model.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshReset(void);

/*----------------------------------------------------------------------------*
 * CSRmeshSetNetworkKey
 */
/*! \brief Set a 128 bit netowrk key
 *
 *  This API sets a 128 bit network key and a 16 bit device Id for 
 *  the local device.
 *
 *  \param networkKey  128 bit network key used to encrypt the outgoing 
 *  MCP data and decrypt the incoming MCP data
 *  \param deviceId    16 bit device id assigned to the local device
 *  \param eventData   On successful completion a network id will be 
 *  available to the app.as a field in this 'result parameter'. This is the
 *  list of the network ids which maps to each network managed by the
 *  device and has direct mapping to all the network keys inside the
 *  core stack.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshSetNetworkKey(CsrUint8 networkKey[CSR_MESH_NETWORK_KEY_SIZE_BYTES],
                    CsrUint16 deviceId, CSR_MESH_APP_EVENT_DATA_T *eventData);


/*----------------------------------------------------------------------------*
 * CSRmeshRemoveNetwork
 */
/*! \brief Remove a network
 *
 *  This API removes the network key associated with the network id from 
 *  the device.
 *
 *  \param netId  Network id of the network to be removed
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshRemoveNetwork(CsrUint8 netId);

#if (CSR_MESH_ON_CHIP != 1)
/*----------------------------------------------------------------------------*
 * CSRmeshSetPassPhrase
 */
/*! \brief Set a passphrase to create a network and network key internally
 *
 *  This API uses a passphrase to generate the network key and also assigns a
 *  16 bit device Id for the local device.
 *
 *  \param passPhrase  The string passphrase using which a NW key will be 
 *  generated internally.
 *  \param length      Lengh of passphrase.
 *  \param deviceId    16 bit device id assigned to the local device.
 *  \param eventData   On successful completion a network id will be
 *  available to the app as a field in this 'result parameter'. This network id
 *  will be a direct mapping to the network key inside.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshSetPassPhrase(CsrUint8* passPhrase, CsrUint8 length, 
                                     CsrUint16 deviceId, CSR_MESH_APP_EVENT_DATA_T *eventData);

/*----------------------------------------------------------------------------*
 * CSRmeshGetNetIDList
 */
/*! \brief Set a passphrase to create a network and network key internally
 *
 *
 *  \param eventData   On successful completion a network id list will be
 *  available to the app as a field in this 'result parameter'. This is the list
 *  of the network ids which maps to each network managed by the device
 *  and has direct mapping to all the network keys inside the core stack.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshGetNetIDList(CSR_MESH_APP_EVENT_DATA_T *eventData);

#endif /* (CSR_MESH_ON_CHIP != 1) */

/*----------------------------------------------------------------------------*
 * CSRmeshGetDeviceID
 */
/*! \brief Gets the 16-bit Device Identifier of the CSRmesh device
 *
 *  \param netId
 *  \param eventData   On successful completion a device id will be available
 *  to the app as a field in this 'result parameter'.
 *
 *  \returns CSRmeshResult. Refer to CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshGetDeviceID( CsrUint8 netId, CSR_MESH_APP_EVENT_DATA_T *eventData);

/*----------------------------------------------------------------------------*
 * CSRmeshGetDeviceUUID
 */
/*! \brief Get the CSRmesh library 128 bit UUID.
 *
 *  \param eventData   On successful completion a uuid id will be available
 *  to the app as a field in this 'result parameter'.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshGetDeviceUUID(CSR_MESH_APP_EVENT_DATA_T *eventData);

/*----------------------------------------------------------------------------*
 * CSRmeshSetTransmitState
 */
/*! \brief CSRmeshSetTransmitState .
 *
 *  This API sets the bearer relay, bearer enable and promiscuous states for
 *  Mesh Bearer layer, also sets relay and promiscuous state of MTL.
 *  MTL and Mesh Bearer layer based on this configuration will allow
 *  reception, transmission of mesh messages and also relay of known 
 *  and unknown mesh messages. Parameters of this API identifies a 16- bit 
 *  bitfield for bearer relay active, bearer enabled and bearer 
 *  promiscuous arguments. 
 *
 *  Note: Setting bit '0' in the corresponding bit mask will disable
 *  the corresponding functionality if it is already active.
 *
 *  \param transmitStateArg      structure containing all the transmit state 
 *  arguments.
 *  \param eventData   On successful completion, transmit state will be 
 *  available to the app.
 *  as a field in this 'result parameter'.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*-----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshSetTransmitState(CSR_MESH_TRANSMIT_STATE_T *transmitStateArg,
                                                          CSR_MESH_APP_EVENT_DATA_T *eventData);

/*----------------------------------------------------------------------------*
 * CSRmeshGetTransmitState
 */
/*! \brief CSRmeshGetTransmitState .
 *
 *  This function reads transmit state arguments of CSRmesh network.
 *
 *  \param netId       Network id of the network for which the relay is 
 *  required to be enabled/disabled.
 *  \param eventData   On successful completion transmit state will be 
 *  available to the app.
 *  as a field in this 'result parameter'.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*-----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshGetTransmitState(CsrUint8 netId, CSR_MESH_APP_EVENT_DATA_T *eventData);

/*----------------------------------------------------------------------------*
 * CSRmeshResetDiagnosticData
 */
/*! \brief Reset the diagnostic data
 *
 *  Applications can use this API to reset all the diagnostic related packet 
 *  counts to zero and reset the time duration as well.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*-----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshResetDiagnosticData(void);

/*----------------------------------------------------------------------------*
 * CSRmeshGetDiagnosticData
 */
/*! \brief Get diagnostic data.
 *
 *  This API gets the diagnostic data captured and stored by MTL.
 *
 *  \param diagnosticMask    The diagnostic mask identifies the type of the 
 *  diagnostic data.
 *  \param eventData         On successful completion a diagnostic data will be
 *  available to the app as a field in this 'result parameter'.
 *  Diagnostic Data(CSR_MESH_DIAGNOSTIC_DATA_T) will be a member
 *  field inside event data and invalid values will be noted using all 0xFF.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*-----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshGetDiagnosticData(CSR_MESH_DIAGNOSTIC_CONFIG_MASK_T diagnosticMask, 
                                                CSR_MESH_APP_EVENT_DATA_T *eventData);

/**************************CSRmesh MASP APIs*******************************/

#if (CSR_MESH_SUPPORT_CONFIG_ROLE == 1)
/*----------------------------------------------------------------------------*
 * CSRmeshStartDeviceInquiry
 */
/*! \brief Configuring device will invoke this to discover new devices.
 *
 *  The configuring device will invoke this API to send MASP_DEVICE_INQUIRY
 *  message to discover new (un-associated) devices. Since it invokes an
 *  asynchronous procedure this API is always a non-blocking method call.
 *  The result parameters (discovered devices) will be available to the 
 *  application asynchronously.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*-----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshStartDeviceInquiry(void);

/*----------------------------------------------------------------------------*
 * CSRmeshSendAssociationRequest
 */
/*! \brief Advertises a CSRmesh device identification message.
 *
 *  Only an application running in a configuring device will invoke this API.
 *  The application will issue this API to add a new device to a network 
 *  identified by netId. The remoteDeviceUUID identifies the new device. 
 *  remoteDeviceAuthCode is the authorisation code of the new device. 
 *  A successful association will ensure that an encrypted (using a 
 *  Diffie-Hellman-Merklegenerated by the internal association procedure) 
 *  remoteDeviceId is distributed to the associated device.
 *
 *  \param netId                Network id of the network to which the device 
 *  is being added.
 *  \param remoteDeviceId       16 bit device id of the device which is being 
 *  added to the network.
 *  \param remoteDeviceHash     32 bit device hash of the remote device
 *  \param remoteDeviceAuthCode the authorisation code of the device 
 *  being added.
 *  \param ttl        Time to live value used for MASP.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshSendAssociationRequest(CsrUint8 netId, CsrUint16 remoteDeviceId, 
                    CsrUint32 remoteDeviceHash, CSR_MESH_AUTH_CODE_T *remoteDeviceAuthCode, CsrUint8 ttl);

#endif
/*----------------------------------------------------------------------------*
 * CSRmeshAssociateToANetwork
 */
/*! \brief Advertises a CSRmesh device identification message.
 *
 *  Application uses this API to appear as a New Device 
 *  (Refer to Section.2.10.1 in Draft E of CS-307193-SP). 
 *  On successful execution of this API the device sends
 *  MASP_DEVICE_IDENTIFICATION message with its 128bit UUID.
 *
 *  \param deviceAppearance   The deviceAppearance of the device. 
 *   This is a pointer to \ref CSR_MESH_DEVICE_APPEARANCE_T structure.
 *  \param ttl        Time to live value used for MASP.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshAssociateToANetwork(CSR_MESH_DEVICE_APPEARANCE_T *deviceAppearance, CsrUint8 ttl);

/*----------------------------------------------------------------------------*
 * CSRmeshSetMeshDuplicateMessageCacheSize
 */
/*! \brief Controls the depth of the duplicate check cache in MTL
 *
 *  Application uses this API to control the depth of message cache in
 *..MTL (dynamically) [0..255]
 *
 *  \param cacheSize      Size of message cache [0 to 255]
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshSetMeshDuplicateMessageCacheSize(CsrUint8 cacheSize);

#if (CSR_MESH_ON_CHIP != 1)
/*----------------------------------------------------------------------------*
 * CSRmeshGenerateMeshIdFromNetId
 */
/*! \brief Generates a MeshId for a given network id
 *
 *  Application uses this API to generate MeshId
 *
 *  \param netId         Network id of the network for which MeshId generation
 *  is required.
 *  \param eventData     On successful completion a secret MeshId be
 *  available to the app.
 *   as a field in this 'result parameter'
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshGenerateMeshIdFromNetId(CsrUint8 netId, 
                                              CSR_MESH_APP_EVENT_DATA_T *eventData);

/*----------------------------------------------------------------------------*
 * CSRmeshGetNetIdFromMeshId
 */
/*! \brief Obtains the network id for a given MeshId
 *
 *  Application uses this API to get the network id for a particular MeshId
 *
 *  \param meshId      Received MeshId required to find the associated
 *  network id.
 *  \param eventData     On successful completion the network id will be
 *..available to the app as a field in this 'result parameter'.
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*----------------------------------------------------------------------------*/
CSRmeshResult CSRmeshGetNetIdFromMeshId(CsrUint8 *meshId, 
                                        CSR_MESH_APP_EVENT_DATA_T *eventData);

#endif /* (CSR_MESH_ON_CHIP != 1) */

/*!@} */
#endif /* __CSR_MESH_H__ */


/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  CSRmesh is a product of Qualcomm Technologies International Ltd.
 */
/*! \file Ping_model.h
 *
 *  \brief Defines CSRmesh Ping Model specific data structures\n
 */
/******************************************************************************/

#ifndef __PING_MODEL_H__
#define __PING_MODEL_H__

#include "csr_mesh_model_common.h"

/*! \addtogroup Ping_Model 
 *  \brief Ping Model API
 *
 * <br>The Ping Model ensures that the MTL can provide reliable message delivery to a peer device. It does this by allowing a transmitting device to send a request that a device supporting the ping model will respond to. The ping model optimises the configuration of devices within the network. The mechanisms to optimise the network are out of scope of this specification.
 * @{
 */

/*============================================================================*
    Public Definitions
 *============================================================================*/

/*! \brief CSRmesh Ping Model message types */

/*! \brief Ping Request: Upon receiving a PING_REQUEST message, the device responds with a PING_RESPONSE message with the TTLAtRx field set to the TTL value from the PING_REQUEST message, and the RSSIAtRx field set to the RSSI value of the PING_REQUEST message. If the bearer used to receive the PING_REQUEST message does not have an RSSI value, then the value 0x00 is used. */
typedef struct
{
    CsrUint8 arbitrarydata[4/size_in_bytes(CsrUint8)]; /*!< \brief Arbitrary data */
    CsrUint8 arbitrarydata_len; /*!< \brief Length of the arbitrarydata field */
    CsrUint8 rspttl; /*!< \brief Response packet time to live */
} CSRMESH_PING_REQUEST_T;

/*! \brief Ping response */
typedef struct
{
    CsrUint8 arbitrarydata[4/size_in_bytes(CsrUint8)]; /*!< \brief Arbitrary data */
    CsrUint8 arbitrarydata_len; /*!< \brief Length of the arbitrarydata field */
    CsrUint8 ttlatrx; /*!< \brief Time to live when received */
    CsrUint8 rssiatrx; /*!< \brief Receiver signal strength when received */
} CSRMESH_PING_RESPONSE_T;


/*!@} */
#endif /* __PING_MODEL_H__ */


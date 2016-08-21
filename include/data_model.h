/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  CSRmesh is a product of Qualcomm Technologies International Ltd.
 */
/*! \file Data_model.h
 *
 *  \brief Defines CSRmesh Data Model specific data structures\n
 */
/******************************************************************************/

#ifndef __DATA_MODEL_H__
#define __DATA_MODEL_H__

#include "csr_mesh_model_common.h"

/*! \addtogroup Data_Model 
 *  \brief Data Model API
 *
 * <br>The Data Model transmits either a stream or blocks of arbitrary data between two devices. For stream data, each octet within the stream has an octet number. This octet number provides the reliable delivery of the stream data as a sequence number. For block data, there is no reliable delivery of messages.<br><br><h4>Data State</h4>The data model has the following state:<ul style='list-style:square;list-style-position: inside;'> <li style='list-style-type:square;'>StreamSequenceNumber</li></ul><br> <h5>StreamSequenceNumber</h5>StreamSequenceNumber is an unsigned 16-bit integer measured in octets. This value determines the octet number for the next message received.<br><br><h4> Data Client Behaviour</h4>Before sending a DATA_STREAM_SEND message, the client should send a DATA_STREAM_FLUSH message to set the current sequence number on the server to a known value.<br><br> When sending a number of octets, the client should split these octets into the minimum number of DATA_STREAM_SEND messages, and send each message in sequence with the appropriate sequence number. The client then awaits the DATA_STREAM_RECEIVED messages to determine if the data has been successfully received and acknowledged. If a DATA_STREAM_RECEIVED message is received and it has a next expected sequence number within the sent octets, then the DATA_STREAM_SEND message containing that octet number should be sent again.<br><br> If no DATA_STREAM_RECEIVED messages are received, then the client should resend the DATA_STREAM_SEND message until the device does acknowledge receipt.<br><br> The client may timeout the reliable delivery of these data stream octets. It should then send a DATA_STREAM_FLUSH message before sending additional stream octets to the server. The length of the time for the timeout is an implementation detail.<br><br> <h6>Note:</h6> The timing of the DATA_STREAM_FLUSH message can be immediately after the timeout occurs, or it can be just before the delivery of the next sequence of octets. This is an implementation detail.
 * @{
 */

/*============================================================================*
    Public Definitions
 *============================================================================*/

/*! \brief CSRmesh Data Model message types */

/*! \brief Flushing Data: Upon receiving a DATA_STREAM_FLUSH message, the device saves the StreamSN field into the StreamSequenceNumber model state and responds with DATA_STREAM_RECEIVED with the StreamNESN field set to the value of the StreamSequenceNumber model state. The device also flushes all partially received stream data from this peer device. */
typedef struct
{
    CsrUint16 streamsn; /*!< \brief Stream sequence number */
} CSRMESH_DATA_STREAM_FLUSH_T;

/*! \brief Sending Data: Upon receiving a DATA_STREAM_SEND message, the device first checks if the StreamSN field is the same as the StreamSequenceNumber model state. If these values are the same, the device passes the StreamOctets field up to the application for processing, and increments StreamSequenceNumber by the length of the StreamOctets field. It then responds with a DATA_STREAM_RECEIVED message with the current value of the StreamSequenceNumber. Note: The DATA_STREAM_RECEIVED message is sent even if the StreamSN received is different from the StreamSequenceNumber state. This allows missing packets to be detected and retransmitted by the sending device. */
typedef struct
{
    CsrUint16 streamsn; /*!< \brief Sequence number of the first octet in StreamOctets. */
    CsrUint8 streamoctets[8/size_in_bytes(CsrUint8)]; /*!< \brief Arbitrary stream of data. */
    CsrUint8 streamoctets_len; /*!< \brief Length of the streamoctets field */
} CSRMESH_DATA_STREAM_SEND_T;

/*! \brief Acknowledgement of data received. */
typedef struct
{
    CsrUint16 streamnesn; /*!< \brief Stream next expected sequence number. */
} CSRMESH_DATA_STREAM_RECEIVED_T;

/*! \brief A block of data, no acknowledgement. Upon receiving a DATA_BLOCK_SEND message, the device passes the DatagramOctets field up to the application for processing. */
typedef struct
{
    CsrUint8 datagramoctets[10/size_in_bytes(CsrUint8)]; /*!< \brief Datagram of octets */
    CsrUint8 datagramoctets_len; /*!< \brief Length of the datagramoctets field */
} CSRMESH_DATA_BLOCK_SEND_T;


/*!@} */
#endif /* __DATA_MODEL_H__ */


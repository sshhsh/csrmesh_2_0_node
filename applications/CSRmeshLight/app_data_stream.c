/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *    app_data_stream.c
 *
 * DESCRIPTION
 *    This file implements a simple protocol over the data model to exchange
 *    device information.
 *    The protocol:
 *       | CODE | LEN (1 or 2 Octets| Data (LEN Octets)|
 *       CODE Defined by APP_DATA_STREAM_CODE_T
 *       LEN - if MS Bit of First octet is 1, then len is 2 octets
 *       if(data[0] & 0x80) LEN = data[0]
 *
 ******************************************************************************/

/*=============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <gatt.h>
#include <timer.h>
#include <mem.h>

/*============================================================================*
 *  CSR Mesh Header Files
 *============================================================================*/
#include <csr_mesh.h>
#include <data_server.h>
#include <data_client.h>

/*=============================================================================*
 *  Local Header Files
*============================================================================*/
#include "app_data_stream.h"

#ifdef ENABLE_DATA_MODEL
/*=============================================================================*
 *  Private Definitions
 *============================================================================*/

#define DEVICE_INFO_STRING               "CSRmesh 2.0 Light\r\n" \
                                         "Supported Models:\r\n" \
                                         " Light Model\r\n" \
                                         " Power Model\r\n" \
                                         " Attention Model\r\n" \
                                         " Battery Model\r\n" \
                                         " Data Model"

/* Data stream send retry wait time */
#define STREAM_SEND_RETRY_TIME            (500 * MILLISECOND)

/* Data stream received timeout value */
#define RX_STREAM_TIMEOUT                 (5 * SECOND)

/* Max number of retries */
#define MAX_SEND_RETRIES                  (3)

/* Max data per per stream send */
#define MAX_DATA_STREAM_PACKET_SIZE       (8)

/*============================================================================*
 *  Private Data Type
 *===========================================================================*/
/* Enum to define different states of the model when sending a stream */
typedef enum
{
    stream_send_idle = 1,     /* No stream is in progress */
    stream_start_flush_sent,  /* Stream_flush sent to start a stream */
    stream_send_in_progress,  /* stream in progress */
    stream_finish_flush_sent  /* stream_finish_flush_sent */
}stream_send_status_t;

/* Enum for different receiver states */
typedef enum
{
    stream_receive_idle = 1,     /* No stream is in progress */
    stream_start_flush_received, /* Received flush to start stream */
    stream_receive_in_progress   /* stream in progress */
}stream_recv_status_t;

typedef struct
{
    uint16 src_id;
    uint16 nesn;  /* Next expected sequence number from the sender */
    stream_recv_status_t status;
}STREAM_RX_T;

typedef struct
{
    uint16 dest_id; /* Data stream destination ID */
    uint16 sn;    /* Sequence number to be sent with next pkt */
    uint16 last_data_len; /* Length of last transmitted stream data */
    stream_send_status_t   status; /* Stream status */
}STREAM_TX_T;

typedef struct
{
    STREAM_RX_T rx;
    STREAM_TX_T tx;
}APP_STREAM_STATE_DATA_T;

/*=============================================================================*
 *  Private Data
 *============================================================================*/
/* String to give a brief description of the application */
static uint8 device_info[256];

/* Device info length */
static uint8 device_info_length;

/* Application data stream state */
static APP_STREAM_STATE_DATA_T app_stream_state;

/* Stream bytes sent tracker */
static uint16 tx_stream_offset = 0;

/* Stream send retry timer */
static timer_id stream_send_retry_tid = TIMER_INVALID;

/* Stream send retry counter */
static uint16 stream_send_retry_count = 0;

/* Current Rx Stream offset */
static uint16 rx_stream_offset = 0;

/* Rx Stream status flag */
static bool rx_stream_in_progress = FALSE;

/* Rx stream timeout tid */
static timer_id rx_stream_timeout_tid;

static APP_DATA_STREAM_CODE_T current_stream_code;


/*=============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
static void streamSendRetryTimer(timer_id tid);
static void sendNextPacket(void);
static void resetRxStreamState(void);
static void rxStreamTimeoutHandler(timer_id tid);
static void handleCSRmeshDataStreamFlushInd(
                                          CSRMESH_DATA_STREAM_FLUSH_T *p_event);
static void handleCSRmeshDataBlockInd(uint16 src_id, 
                                            CSRMESH_DATA_BLOCK_SEND_T *p_event);
static void handleCSRmeshDataStreamDataInd(uint16 src_id, 
                                           CSRMESH_DATA_STREAM_SEND_T *p_event);
static void handleCSRmeshDataStreamSendCfm(
                                       CSRMESH_DATA_STREAM_RECEIVED_T *p_event);
static void startStream(uint16 dest_id);
static void endStream(void);

/*=============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      streamSendRetryTimer
 *
 *  DESCRIPTION
 *      Timer handler to retry sending next packet
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void streamSendRetryTimer(timer_id tid)
{
    CSRMESH_DATA_STREAM_SEND_T send_param;
    if( tid == stream_send_retry_tid )
    {
        stream_send_retry_tid = TIMER_INVALID;
        stream_send_retry_count++;
        if( stream_send_retry_count < MAX_SEND_RETRIES )
        {
            MemCopy(send_param.streamoctets, &device_info[tx_stream_offset],
                                             app_stream_state.tx.last_data_len);
            send_param.streamoctets_len = app_stream_state.tx.last_data_len;
            send_param.streamsn = app_stream_state.tx.sn;
                

            /* Send the next packet */
            DataStreamSend(CSR_MESH_DEFAULT_NETID, 
                                      app_stream_state.tx.dest_id, &send_param);

            stream_send_retry_tid =  TimerCreate(STREAM_SEND_RETRY_TIME, TRUE,
                                                          streamSendRetryTimer);
        }
        else
        {
            stream_send_retry_count = 0;
            endStream();
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      rxStreamTimeoutHandler
 *
 *  DESCRIPTION
 *      Timer handler to handle rx stream timeout
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void rxStreamTimeoutHandler(timer_id tid)
{
    if( tid == rx_stream_timeout_tid )
    {
        /* Reset the stream */
        rx_stream_timeout_tid = TIMER_INVALID;
        rx_stream_in_progress = FALSE;
        resetRxStreamState();

    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      sendNextPacket
 *
 *  DESCRIPTION
 *      Forms a stream data packet with the current counter and sends it to
 *      the stream receiver
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void sendNextPacket(void)
{
    uint16 data_pending, len;
    CSRMESH_DATA_STREAM_SEND_T send_param;
    /* Stop retry timer */
    stream_send_retry_count = 0;
    TimerDelete(stream_send_retry_tid);
    stream_send_retry_tid = TIMER_INVALID;

    data_pending = device_info_length+2 - tx_stream_offset;

    if( data_pending )
    {
        len = (data_pending > MAX_DATA_STREAM_PACKET_SIZE)? 
                                MAX_DATA_STREAM_PACKET_SIZE : data_pending;

        MemCopy(send_param.streamoctets, &device_info[tx_stream_offset],len);
        send_param.streamoctets_len = len;
        send_param.streamsn = app_stream_state.tx.sn;
                
        /* Send the next packet */
        DataStreamSend(CSR_MESH_DEFAULT_NETID, app_stream_state.tx.dest_id, 
                                                                  &send_param);
        app_stream_state.tx.last_data_len = len;

        stream_send_retry_tid = TimerCreate(STREAM_SEND_RETRY_TIME, TRUE,
                                                       streamSendRetryTimer);
    }
    else
    {
        /* Send flush to indicate end of stream */
        endStream();
    }
}

/*----------------------------------------------------------------------------*
 * NAME 
 *     resetRxStreamState
 * 
 * DESCRIPTION
 *     Resets a stream being received
 *     This function resets a stream being received.\n
 *     The application must call this function to reset a stream in progress
 *     in case of a timeout. This will reset the receive status of the stream
 *     model so that it is ready to receive a new stream.
 *----------------------------------------------------------------------------*/
static void resetRxStreamState(void)
{
    /* Reset source ID */
    app_stream_state.rx.src_id = 0;
    app_stream_state.rx.status = stream_receive_idle;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      handleCSRmeshDataStreamFlushInd
 *
 *  DESCRIPTION
 *      This function handles the CSR_MESH_DATA_STREAM_FLUSH message.
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void handleCSRmeshDataStreamFlushInd(
                                        CSRMESH_DATA_STREAM_FLUSH_T *p_event)
{
    rx_stream_offset = 0;

    if( rx_stream_in_progress == FALSE )
    {
        /* Start the stream timeout timer */
        TimerDelete(rx_stream_timeout_tid);
        rx_stream_timeout_tid = TimerCreate(RX_STREAM_TIMEOUT, TRUE,
                                                        rxStreamTimeoutHandler);
    }
    else
    {
        /* End of stream */
        rx_stream_in_progress = FALSE;
        TimerDelete(rx_stream_timeout_tid);
        rx_stream_timeout_tid = TIMER_INVALID;
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      handleCSRmeshDataBlockInd
 *
 *  DESCRIPTION
 *      This function handles the CSR_MESH_DATA_BLOCK_IND message
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void handleCSRmeshDataBlockInd(uint16 src_id, 
                                            CSRMESH_DATA_BLOCK_SEND_T *p_event)
{
    switch(p_event->datagramoctets[0])
    {
        case CSR_DEVICE_INFO_REQ:
        {
            /* Set the source device ID as the stream target device */
            tx_stream_offset = 0;
            /* Set the opcode to CSR_DEVICE_INFO_RSP */
            device_info[0] = CSR_DEVICE_INFO_RSP;

            /* start sending the data */
            startStream(src_id);
        }
        break;

        case CSR_DEVICE_INFO_RESET:
        {
            /* Reset the device info */
            device_info_length = sizeof(DEVICE_INFO_STRING);
            device_info[0] = CSR_DEVICE_INFO_RSP;
            device_info[1] = device_info_length;
            MemCopy(&device_info[2], DEVICE_INFO_STRING,
                                                   sizeof(DEVICE_INFO_STRING));
        }
        break;

        default:
        break;
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      handleCSRmeshDataStreamDataInd
 *
 *  DESCRIPTION
 *      This function handles the CSR_MESH_DATA_STREAM_DATA_IND message
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void handleCSRmeshDataStreamDataInd(uint16 src_id, 
                                            CSRMESH_DATA_STREAM_SEND_T *p_event)
{
    /* Restart the stream timeout timer */
    TimerDelete(rx_stream_timeout_tid);
    rx_stream_timeout_tid = TimerCreate(RX_STREAM_TIMEOUT, TRUE,
                                                        rxStreamTimeoutHandler);

    /* Set stream_in_progress flag to TRUE */
    rx_stream_in_progress = TRUE;

    if( rx_stream_offset == 0 )
    {
        /* If the stream offset is 0. The data[0] will be the CODE */
        switch(p_event->streamoctets[0])
        {
            case CSR_DEVICE_INFO_REQ:
            {
                /* Set the source device ID as the stream target device */
                tx_stream_offset = 0;
                /* Set the stream code to CSR_DEVICE_INFO_RSP */
                device_info[0] = CSR_DEVICE_INFO_RSP;
                /* Start the stream */
                startStream(src_id);
            }
            break;

            case CSR_DEVICE_INFO_RESET:
            {
                /* Reset the device info */
                device_info_length = sizeof(DEVICE_INFO_STRING);
                device_info[0] = CSR_DEVICE_INFO_RSP;
                device_info[1] = device_info_length;
                MemCopy(&device_info[2], DEVICE_INFO_STRING,
                                                    sizeof(DEVICE_INFO_STRING));
            }
            break;

            case CSR_DEVICE_INFO_SET:
            {
                /* CSR_DEVICE_INFO_SET is received. Store the code, length and
                 * the data into the device_info array in the format received
                 */
                current_stream_code = CSR_DEVICE_INFO_SET;
                device_info_length = p_event->streamoctets[1];
                MemCopy(device_info, p_event->streamoctets,
                                                    p_event->streamoctets_len);
                rx_stream_offset = p_event->streamoctets_len;
            }
            break;
            default:
            break;
        }
    }
    else
    {
        if( current_stream_code == CSR_DEVICE_INFO_SET &&
            rx_stream_offset + p_event->streamoctets_len < sizeof(device_info))
        {
            MemCopy(&device_info[rx_stream_offset], p_event->streamoctets,
                                                    p_event->streamoctets_len);
            rx_stream_offset += p_event->streamoctets_len;
        }

        /* No other CODE is handled currently */
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      handleCSRmeshDataStreamSendCfm
 *
 *  DESCRIPTION
 *      This function handles the CSR_MESH_DATA_STREAM_SEND_CFM message
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void handleCSRmeshDataStreamSendCfm(
                                    CSRMESH_DATA_STREAM_RECEIVED_T *p_event)
{
    
    tx_stream_offset += app_stream_state.tx.last_data_len;
    /* Send next block if it is not end of string */
    sendNextPacket();
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      startStream
 *
 *  DESCRIPTION
 *      Initialises the stream model to start sending a data stream. 
 *      This function sets the receiver device ID to which the data is to be
 *      sent using the StreamSendData
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void startStream(uint16 dest_id)
{
    CSRMESH_DATA_STREAM_FLUSH_T flush_param;
    app_stream_state.tx.dest_id = dest_id;

    /* Initialise the next expected sequence number to 0 */
    app_stream_state.tx.sn = 0;
    
    app_stream_state.tx.status = stream_start_flush_sent;
    app_stream_state.tx.last_data_len = 0;

    /* Send flush to indicate start of stream */
    flush_param.streamsn = app_stream_state.tx.sn;
    DataStreamFlush(CSR_MESH_DEFAULT_NETID, dest_id, &flush_param);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      endStream
 *
 *  DESCRIPTION
 *      Sends flush to end stream and updates the stream transmit state
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void endStream(void)
{
    CSRMESH_DATA_STREAM_FLUSH_T flush_param;
    if(app_stream_state.tx.status == stream_send_in_progress)
    {
        app_stream_state.tx.status = stream_finish_flush_sent;
        app_stream_state.tx.last_data_len = 0;
    }
    /* Send flush to end stream */
    flush_param.streamsn = app_stream_state.tx.sn;
    DataStreamFlush(CSR_MESH_DEFAULT_NETID, app_stream_state.tx.dest_id, 
                                                                  &flush_param);
}


/*=============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppDataStreamInit
 *
 *  DESCRIPTION
 *      This function initializes the stream Model.
 *
 *  RETURNS
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
extern void AppDataStreamInit(uint16 *group_id_list, uint16 num_groups)
{
    /* Register both both data client and server as we support both send
     * and receive stream
     */
    /* Server */
    DataModelInit(0, group_id_list, num_groups, AppDataServerHandler);
    /* Client */
    DataModelClientInit(AppDataClientHandler);

    /* Reset timers */
    stream_send_retry_tid = TIMER_INVALID;
    rx_stream_timeout_tid = TIMER_INVALID;

    /* Reset the device info */
    device_info_length = sizeof(DEVICE_INFO_STRING);
    device_info[0] = CSR_DEVICE_INFO_RSP;
    device_info[1] = device_info_length;

    /* Initialise stream state */
    app_stream_state.rx.src_id = 0;
    app_stream_state.rx.status = stream_receive_idle;

    app_stream_state.tx.dest_id = 0;
    app_stream_state.tx.status = stream_send_idle;
    app_stream_state.tx.last_data_len = 0;

    MemCopy(&device_info[2], DEVICE_INFO_STRING, sizeof(DEVICE_INFO_STRING));
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      AppDataServerHandler
 *
 *  DESCRIPTION
 *      Application function to handle CSRmesh Data Model messages
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
CSRmeshResult AppDataServerHandler(CSRMESH_MODEL_EVENT_T event_code, 
                                   CSRMESH_EVENT_DATA_T* p_event,
                                   CsrUint16 length, void **state_data)
{
    uint16 sn;
    uint16 send_ack;
    uint16 b_use_msg = FALSE;

    switch(event_code)
    {
        /* Stream flush indication */
        case CSRMESH_DATA_STREAM_FLUSH:
        {
            CSRMESH_DATA_STREAM_FLUSH_T *p_flush_msg = 
                (CSRMESH_DATA_STREAM_FLUSH_T *)p_event->data;

            sn = p_flush_msg->streamsn;

            /*  If the state is no-stream, update nesn and the sender ID */
            if( app_stream_state.rx.status == stream_receive_idle)
            {
                /* The stream end flush could have been re-transmitted. 
                 * Ack the message. Move to a new stream only if the sn or the 
                 * rx.src_id does not match
                 */
                send_ack = TRUE;
                if( app_stream_state.rx.nesn != sn ||
                    app_stream_state.rx.src_id != p_event->src_id)
                {
                    app_stream_state.rx.status = stream_start_flush_received;
                    app_stream_state.rx.src_id = p_event->src_id;
                    app_stream_state.rx.nesn = sn;
                    b_use_msg = TRUE;
                }
            }
            else if(app_stream_state.rx.status == stream_start_flush_received)
            {
                /* We have already received a flush to start a stream. 
                 * Respond to the message if it is from the same source. 
                 * Ignore otherwise. App is already notified no need to notify
                 * again
                 */
                if( app_stream_state.rx.src_id == p_event->src_id)
                {
                    send_ack = TRUE;
                }
            }
            else if(app_stream_state.rx.status == stream_receive_in_progress)
            {
                /* Data stream already in progress. Check if the sn is equal to
                 * nesn and the sender is rx.src_id. Otherwise ignore flush
                 */
                if(p_event->src_id == app_stream_state.rx.src_id &&
                   sn == app_stream_state.rx.nesn )
                {
                    /* End of stream */
                    send_ack = TRUE;
                    b_use_msg = TRUE;
                    app_stream_state.rx.status = stream_receive_idle;
                }
            }

            *state_data = NULL;
            if(send_ack == TRUE)
            {
                /* Acknowledge the sender */
                *state_data = &app_stream_state.rx.nesn;
            }
            
            if( b_use_msg == TRUE)
            {
                /* Message is useful app */
                handleCSRmeshDataStreamFlushInd(p_flush_msg);
            }
        }
        break;

        case CSRMESH_DATA_STREAM_SEND:
        {
            CSRMESH_DATA_STREAM_SEND_T *p_stream_msg =
                                (CSRMESH_DATA_STREAM_SEND_T *)p_event->data;

            /* Accept only if a data stream is received for an on-going 
             * stream from rx.src_id
             */
            
            if(p_event->src_id == app_stream_state.rx.src_id )
            {
                app_stream_state.rx.status = stream_receive_in_progress;
                sn = p_stream_msg->streamsn;

                if( sn == app_stream_state.rx.nesn )
                {
                    /* Update nesn */
                    app_stream_state.rx.nesn += p_stream_msg->streamoctets_len;

                    /* Process data */
                    handleCSRmeshDataStreamDataInd(p_event->src_id,
                                                   p_stream_msg);
                }

                /* Acknowledge the sender */
                *state_data = &app_stream_state.rx.nesn;
            }
            else
            {
                /* A stream transfer is ongoing with another device. Or 
                 * a Flush to start a stream was not received.
                 * Ignore the data stream send.
                 */
                *state_data = NULL;
            }
        }
        break;
        /* Received a single block of data */
        case CSRMESH_DATA_BLOCK_SEND:
        {
            CSRMESH_DATA_BLOCK_SEND_T *p_block_send =
                (CSRMESH_DATA_BLOCK_SEND_T *)p_event->data;
            handleCSRmeshDataBlockInd(p_event->src_id, p_block_send);
        }
        break;

        default:
        break;
    }
    return CSR_MESH_RESULT_SUCCESS;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppDataClientHandler
 *
 *  DESCRIPTION
 *      Application function to handle CSRmesh Data Model messages
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
CSRmeshResult AppDataClientHandler(CSRMESH_MODEL_EVENT_T event_code, 
                                   CSRMESH_EVENT_DATA_T* p_event,
                                   CsrUint16 length,
                                   void **state_data)
{
    uint16 nesn;

    switch(event_code)
    {
        /* Received a repsonse to a flush or a data_stream */
        case CSRMESH_DATA_STREAM_RECEIVED:
        {
            CSRMESH_DATA_STREAM_RECEIVED_T *p_data_rcvd =
                      (CSRMESH_DATA_STREAM_RECEIVED_T *)p_event->data;
            /* Handle the message only if it is addressed to self device ID */
            /* Get the nesn from ack */
            nesn = p_data_rcvd->streamnesn;

            if(app_stream_state.tx.status == stream_start_flush_sent &&
               nesn == app_stream_state.tx.sn)
            {
                /* Received the acknowledgement for the stream flush
                 * sent to start streaming
                 */
                app_stream_state.tx.status = stream_send_in_progress;

                /* If there is any stream packet pending, send it */
                handleCSRmeshDataStreamSendCfm(p_data_rcvd);
            }
            else if(app_stream_state.tx.status == stream_finish_flush_sent &&
               nesn == app_stream_state.tx.sn)
            {
                /* Received the acknowledgement for the stream flush sent
                 * to finish the stream
                 */
                app_stream_state.tx.status = stream_send_idle;
                app_stream_state.tx.sn = 0;
            }
            
            /* nesn must be tx.sn + tx.last_data_len */
            else if(app_stream_state.tx.status == stream_send_in_progress &&
                    nesn == app_stream_state.tx.sn +
                                  app_stream_state.tx.last_data_len)
            {
                app_stream_state.tx.sn += app_stream_state.tx.last_data_len;
                /* If there is any stream packet pending, send it */
                handleCSRmeshDataStreamSendCfm(p_data_rcvd);
            }
        }
        break;

        default:
        break;
    }
    return CSR_MESH_RESULT_SUCCESS;
}

#endif


 /******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *****************************************************************************/
/*! \file csr_mesh_types.h
 *  \brief CSRmesh library data types
 *
 *   This file contains the different data types used in CSRmesh library
 *
 *   NOTE: This library includes the Mesh Transport Layer, Mesh Control
 *   Layer and Mesh Association Layer functionality.
 */
 /*****************************************************************************/
#ifndef _CSR_MESH_TYPES_H_
#define _CSR_MESH_TYPES_H_

#include "csr_types.h"
#include "csr_mesh_config.h"
#include "csr_mesh_result.h"

#if defined(CSR101x) || defined(CSR101x_A05)
#include "ls_types.h"
#endif

/*! \addtogroup CSRmesh
 * @{
 */
 
/*============================================================================*
Public Definitions
*============================================================================*/

/*! \brief Number of timers required for CSRmesh library to be reserved by
 * the application.
 */
#define CSR_MESH_MAX_NO_TIMERS         (7) /*!< \brief User application needs
                                          * to reserve these many timers along
                                          * with application timers. Required
                                          * for CSR1010 only.\n
                                          * Example:\n
                                          * \code #define MAX_APP_TIMERS (3 + CSR_MESH_MAX_NO_TIMERS)
                                          * \endcode
                                          */

/*! \brief Number of words required by the CSRMesh library to save the internal
 *   configuration values on the NVM. Required for CSR1010 only.
 */
#define CSR_MESH_NVM_SIZE                    (22 + (CSR_MESH_MAX_NO_OF_NWKS* 10 /* Network info nvm size */))


/*! \brief Offset used for storing device UUID in NVM 
 */
#define CSR_MESH_NVM_DEVICE_UUID_OFFSET      (1)

/*! \brief Offset used for storing authorization code in NVM 
*/
#define CSR_MESH_NVM_DEVICE_AUTH_CODE_OFFSET (9)

/*! \brief Invalid network id
 */
#define CSR_MESH_INVALID_NWK_ID         (0xFF)

/*! \brief short name for the device */
#define CSR_MESH_SHORT_NAME_LENGTH      (9)

/*! \brief This constant is used in the main server app to define array that is 
 *  large enough to hold the advertisement data.
 */
#define MAX_ADV_DATA_LEN                        (31)
/*! \brief CSRmesh Network key in bytes */
#define CSR_MESH_NETWORK_KEY_SIZE_BYTES         (16)
/*!\brief CSRmesh Mesh_Id size in bytes */
#define CSR_MESH_MESH_ID_SIZE_BYTES             (16)

/*!\brief LE Bearer Type */
#define LE_BEARER_ACTIVE                        (1)
/*!\brief GATT Bearer Type */
#define GATT_SERVER_BEARER_ACTIVE               (2)


/*! \brief Flag determining the type of the device */
typedef enum
{
    CSR_MESH_CONFIG_DEVICE                    = 0, /*!< \brief CSRmesh configuring device type */
    CSR_MESH_NON_CONFIG_DEVICE                = 1, /*!< \brief CSRmesh non-configuring device type */
    CSR_MESH_CONFIG_DEVICE_WITH_AUTH_CODE     = 2, /*!<\brief CSRmesh config device with auth code type */
    CSR_MESH_NON_CONFIG_DEVICE_WITH_AUTH_CODE = 3, /*!<\brief CSRmesh non-config device with auth code type */
} CSR_MESH_CONFIG_FLAG_T;

/*! \brief Operation status passed with App-Callback to indicate the result of an
 *  asynchronous operation or to inform the App that the callback is mader to request info
 */
typedef enum
{
    CSR_MESH_OPERATION_SUCCESS                = 0x00, /*!< \brief Operation status success  */
    CSR_MESH_OPERATION_STACK_NOT_INITIALIZED  = 0x01, /*!< \brief Operation status stack not initialized */
    CSR_MESH_OPERATION_NOT_PERMITTED          = 0x02, /*!< \brief Operation status operation not permited */
    CSR_MESH_OPERATION_MEMORY_FULL            = 0x03, /*!< \brief Operation status memory full */
    CSR_MESH_OPERATION_REQUEST_FOR_INFO       = 0x04, /*!< \brief Operation status request for info to app */
    CSR_MESH_OPERATION_GENERIC_FAIL           = 0xFF, /*!< \brief Operation status generic fail */
} CSR_MESH_OPERATION_STATUS_T;

/******************************************************************************/

/*! \brief 128-bit UUID type */
typedef struct
{
    CsrUint8 uuid[16]; /*!< \brief CSRmesh 128-bit UUID */
} CSR_MESH_UUID_T;

/*! \brief CSRmesh Product ID, Vendor ID and Version Information. */
typedef struct
{
    CsrUint16 vendor_id;   /*!< \brief Vendor Identifier.  */
    CsrUint16 product_id;  /*!< \brief Product Identifier. */
    CsrUint32 version;     /*!< \brief Version Number.     */
} CSR_MESH_VID_PID_VERSION_T;

/*! \brief 64 bit Authorisation Code type */
typedef struct
{
    CsrUint8 auth_code[8]; /*!< \brief CSRmesh 64 bit Authorisation Code */
}CSR_MESH_AUTH_CODE_T;

/*! \brief CSRmesh Scan and Advertising Parameters */
typedef struct
{
    CsrUint16 scan_duty_cycle;      /*!< \brief CSRmesh scan duty cycle (0 - 100 percent) */
    CsrUint16 advertising_interval; /*!< \brief CSRmesh advertising interval in milliseconds */
    CsrUint16 advertising_time;     /*!< \brief CSRmesh advertising time  in milliseconds */
    CsrInt8 tx_power;                /*!< \brief CSRmesh tx_power */
}CSR_MESH_CONFIG_BEARER_PARAM_T;


/*! \brief CSRmesh Message types */
typedef enum
{
    CSR_MESH_MESSAGE_ASSOCIATION, /*!< \brief CSRmesh Association message. */
    CSR_MESH_MESSAGE_CONTROL      /*!< \brief CSRmesh Control message. */
} CSR_MESH_MESSAGE_T;

/*! \brief CSRmesh event types */
typedef enum
{
    CSR_MESH_INIT_EVENT                    = 0x0001,    /*!< \brief Type Mesh stack init event */
    CSR_MESH_REGISTER_APP_CB_EVENT         = 0x0002,    /*!< \brief Type Register-App event */
    CSR_MESH_RESET_EVENT                   = 0x0003,    /*!< \brief Type Reset event */
    CSR_MESH_START_EVENT                   = 0x0004,    /*!< \brief Type Start event */
    CSR_MESH_STOP_EVENT                    = 0x0005,    /*!< \brief Type Stop event */
    CSR_MESH_TRANSMIT_STATE_EVENT          = 0x0006,    /*!< \brief Type Bearer State event */
    CSR_MESH_START_DEVICE_INQUIRY_EVENT    = 0x0007,    /*!< \brief Type Device Enquiry event */
    CSR_MESH_ASSOC_STARTED_EVENT           = 0x0008,    /*!< \brief Type Assoc Started event */
    CSR_MESH_ASSOC_COMPLETE_EVENT          = 0x0009,    /*!< \brief Type Assoc Complete event */
    CSR_MESH_SEND_ASSOC_COMPLETE_EVENT     = 0x000A,    /*!< \brief Type Send Assoc Complete event */
    CSR_MESH_GET_DEVICE_ID_EVENT           = 0x000B,    /*!< \brief Type Get Device Id event */
    CSR_MESH_GET_DEVICE_UUID_EVENT         = 0x000C,    /*!< \brief Type Get Device Uuid event */
    CSR_MESH_MASP_DEVICE_IND_EVENT         = 0x000D,    /*!< \brief Type Masp Device Ind event */
    CSR_MESH_MASP_DEVICE_APPEARANCE_EVENT  = 0x000E,    /*!< \brief Type Masp Device Appearance event */
    CSR_MESH_NETWORK_ID_LIST_EVENT         = 0x000F,    /*!< \brief Type Network Id List event */
    CSR_MESH_SET_MAX_NO_OF_NETWORK_EVENT   = 0x0010,    /*!< \brief Type Max No of Network event */
    CSR_MESH_SET_PASSPHRASE_EVENT          = 0x0011,    /*!< \brief Type Set Passphrase event */
    CSR_MESH_SET_NETWORK_KEY_EVENT         = 0x0012,    /*!< \brief Type Set Network Key event */
    CSR_MESH_CONFIG_RESET_DEVICE_EVENT     = 0x0013,    /*!< \brief Type Config Reset Device event */
    CSR_MESH_CONFIG_SET_PARAMS_EVENT       = 0x0014,    /*!< \brief Type Config Set Params event */
    CSR_MESH_CONFIG_GET_PARAMS_EVENT       = 0x0015,    /*!< \brief Type Config Get Params event */
    CSR_MESH_GET_VID_PID_VERSTION_EVENT    = 0x0016,    /*!< \brief Type Get Vid-Pid Version event */
    CSR_MESH_GET_DEVICE_APPEARANCE_EVENT   = 0x0017,    /*!< \brief Type Get Device Appearance event */
    CSR_MESH_GROUP_SET_MODEL_GROUPID_EVENT = 0x0018,    /*!< \brief Type Group Set Model Group-id event */
    CSR_MESH_SEND_RAW_MCP_MSG_EVENT        = 0x0019,    /*!< \brief Type Send Raw MCP Msg event */
    CSR_MESH_SEND_MCP_MSG_EVENT            = 0x001A,    /*!< \brief Type Send MCP Msg event */
    CSR_MESH_MCP_REGISTER_MODEL_EVENT      = 0x001B,    /*!< \brief Type MCP Register Model event */
    CSR_MESH_MCP_REGISTER_MODEL_CLIENT_EVENT = 0x001C,  /*!< \brief Type MCP Register Client Model event */
    CSR_MESH_REMOVE_NETWORK_EVENT          = 0x001D,    /*!< \brief Type Remove Network event */
    CSR_MESH_GET_DIAG_DATA_EVENT           = 0x001E,    /*!< \brief Type Get Diagnostic Data event */
    CSR_MESH_RESET_DIAG_DATA_EVENT         = 0x001F,    /*!< \brief Type Reset Diagnostic Data event */
    CSR_MESH_REGISTER_SNIFFER_APP_CB_EVENT = 0x0020,    /*!< \brief Type Register Sniffer App Cb event */
    CSR_MESH_GET_MESH_ID_EVENT             = 0x0021,    /*!< \brief Type Get Mesh Id Data event */
    CSR_MESH_GET_NET_ID_FROM_MESH_ID_EVENT = 0x0022,    /*!< \brief Type Get Network id from Mesh Id event */
    CSR_MESH_ASSOCIATION_ATTENTION_EVENT   = 0x0023,    /*!< \brief Type Association attention event */
    CSR_MESH_BEARER_STATE_EVENT            = 0x0024,    /*!< \brief Type Bearer state is updated */    
    CSR_MESH_INVALID_EVENT                 = 0xFFFF     /*!< \brief Type Invalid event */
} CSR_MESH_EVENT_T;

/*! \brief CSRmesh Diagnostic duplicate message count types */
typedef enum
{
    CSR_MESH_DIAGNOSTIC_DUPLICATE_MSG_COUNT = 0x01, /*!< \brief  Duplicate message count mask. */
    CSR_MESH_DIAGNOSTIC_UNKNOWN_MSG_COUNT   = 0x02, /*!< \brief Unknown message count mask.   */
    CSR_MESH_DIAGNOSTIC_KNOWN_MSG_COUNT     = 0x04,/*!< \brief  Known message count mask.     */
    CSR_MESH_DIAGNOSTIC_RELAYED_MSG_COUNT   = 0x08, /*!< \brief  Relayed message count mask.   */
#if (CSR_MESH_ON_CHIP != 1)
    CSR_MESH_DIAGNOSTIC_BEARER_SPECIFIC_RX  = 0x10,  /*!< \brief Bearer specific Rx. */
    CSR_MESH_DIAGNOSTIC_BEARER_SPECIFIC_TX  = 0x20,/*!< \brief  Bearer specific Tx */
#endif /* (CSR_MESH_ON_CHIP != 1) */
    CSR_MESH_DIAGNOSTIC_ALL                 = 0xFF, /*!< \brief  All message count mask.       */
} CSR_MESH_DIAGNOSTIC_CONFIG_MASK_T;


/*! \brief CSR Mesh Network Id List */
typedef struct
{
    /*!< \brief List of existing mesh network IDs. Each octet 
     * value identifies one distinguished mesh network. This is 
     * defined as a flexible array and a field of this type will 
     * be part of union CSR_MESH_APP_CB_DATA_T. The array netIdList
     * will have maximum number of array elements i.e. allowed by
     * the maximum size of the union. An invalid network id is 
     * defined as 0xFF.
     */
    CsrUint8 length; /*!< \brief length of netid list */
    CsrUint8 netIDList[CSR_MESH_MAX_NO_OF_NWKS]; /*!< \brief list of Net-ids */
} CSR_MESH_NETID_LIST_T;

/*! \brief CSR Mesh transmit state */
typedef struct
{
    CsrUint8 netId;/*!< \brief Nwk id for which realy is enable */
    CsrBool relayEnable;/*!< \brief realy information */
} CSR_MESH_MCP_TRANSMIT_INFO_T;

/*! \brief CSR Mesh transmit state */
typedef struct
{
    CsrUint16 bearerRelayActive;/*!< \brief Bearer realy information */
    CsrUint16 bearerEnabled;/*!< \brief Bearer Enable information */
    CsrUint16 bearerPromiscuous;/*!< \brief Bearer Promiscuous information */
    CSR_MESH_MCP_TRANSMIT_INFO_T mcpRelayInfo;/*!< \brief Mcp realy information */
    CsrBool maspRelayEnable;/*!< \brief Masp realy information */
    CsrBool promiscuousEnable;/*!< \brief Promiscous enable information */
} CSR_MESH_TRANSMIT_STATE_T;

/*! \brief CSR Mesh TTL */
typedef struct
{
    CSR_MESH_MESSAGE_T msgType; /*!< \brief Msg type - Whether MASP or MCP */
    CsrUint8               ttl; /*!< \brief TTL Value */
} CSR_MESH_TTL_T;

/*! \brief CSRmesh Device Appearance. The Appearance is a 24-bit value that is 
 *   composed of an "organization" and an "organization appearance". 
 */
typedef struct
{
    CsrUint8  organization; /*!< \brief Identifies the organization which assigns
                               device appearance values */
    CsrUint16 value;        /*!< \brief Appearance value */
    CsrUint32 deviceHash;  /*!<\brief deviceHash of remote device */
}CSR_MESH_APPEARANCE_T;

/*! \brief Device Appearance data type */
typedef struct
{
    CsrUint8 shortName[CSR_MESH_SHORT_NAME_LENGTH]; /*!< \brief short name of the device */
    CSR_MESH_APPEARANCE_T appearance; /*!< \brief deviceApperance of the device */
}CSR_MESH_DEVICE_APPEARANCE_T;

/*! \brief Device indication data type */
typedef struct 
{
    CsrUint32         deviceHash; /*!<\brief deviceHash of remote device */
    CSR_MESH_UUID_T   uuid; /*!< \brief CSRmesh 128-bit UUID of remote device*/
}CSR_MESH_MASP_DEVICE_IND_T;

/*! \brief Association attention request data type */
typedef struct
{
    CsrUint8 attract_attention; /*!< \brief Enable or disable attracting attention\n
                                 *          0 - Do not attract attention\n
                                 *          1 - Attract attention
                                 *    Refer to \ref XAP_8bit_handler_page
                                */
    CsrUint16 duration;         /*!< \brief Duration for which the attention is 
                                 *          requested. 
                                 */
} CSR_MESH_ASSOCIATION_ATTENTION_DATA_T;

/*! \brief CSR Mesh Group Id Related Data - To provide to app while handling Group model data in core mesh */
typedef struct
{
    CsrUint8              netId; /*!< \brief network Id of the nwk for 
                                       which the group is set */
    CsrUint8              model; /*!< \brief MCP Model No */
    CsrUint8              gpIdx;     /*!< \brief Group Idx */
    CsrUint8           instance;     /*!< \brief Group Instance */
    CsrUint16              gpId;     /*!< \brief Group Id */
} CSR_MESH_GROUP_ID_RELATED_DATA_T;

/*! \brief CSR Mesh Bearer state Type */
typedef struct 
{
    CsrUint16 bearerRelayActive; /*!< \brief Mask of bearer Relay state.    */
    CsrUint16 bearerEnabled;     /*!< \brief Mask of bearer Active state.   */
    CsrUint16 bearerPromiscuous; /*!< \brief Mask of promiscuous mode. */
} CSR_MESH_BEARER_STATE_DATA_T;

#if (CSR_MESH_ON_CHIP != 1)
/*! \brief CSR Mesh Bearer specific diagnostic data Type */
typedef struct
{
    CsrUint16 bearerMask; /*!< \brief bitmask specifying bearers */
    CsrUint32 numRxMeshMsg[CSR_MESH_MAX_NO_OF_SUPPORTED_BEARER]; /*!< \brief num of mesh msg rcvd in a particular bearer */
    CsrUint32 numTxMeshMsg[CSR_MESH_MAX_NO_OF_SUPPORTED_BEARER]; /*!< \brief num of mesh msg txed in a particular bearer */
} CSR_MESH_BEARER_SPECIFIC_DIAGNOSTIC_DATA_T;
#endif /* (CSR_MESH_ON_CHIP != 1) */

/*! \brief CSR Mesh Diagnostic data Type */
typedef struct
{
    CsrUint32 numDuplicateMsg;      /*!< \brief Duplicate message count */
    CsrUint32 numUnknownMsg;        /*!< \brief Unknown message count */
    CsrUint32 numKnownMsg;          /*!< \brief Known message count */
    CsrUint32 numRelayedMsg;        /*!< \brief Relayed message count */
    CsrUint32 diagnosisDuration;    /*!< \brief Duration indicates the time interval
                                        in terms of milliseconds during which
                                        the diagnostic data is captured. */
#if (CSR_MESH_ON_CHIP != 1)
    CSR_MESH_BEARER_SPECIFIC_DIAGNOSTIC_DATA_T bearerSpfDiagnostic; /*!< \brief Bearer specific diagnostic summary */
#endif /* #ifndef CSR_MESH_ON_CHIP */
} CSR_MESH_DIAGNOSTIC_DATA_T;

/*!\brief CSR Mesh mesh_id data Type */
typedef struct
{
    CsrUint8 meshId[CSR_MESH_MESH_ID_SIZE_BYTES];/*!< \brief CSR Mesh mesh_id */
} CSR_MESH_MESH_ID_DATA_T;

/*! \brief CSR Mesh App Event Data - to provide event, status * app data to app */
typedef struct
{
    CSR_MESH_EVENT_T              event;  /*!< \brief CSR Mesh Event */
    CSR_MESH_OPERATION_STATUS_T   status;  /*!< \brief CSR Mesh Operation Status */
    void *appCallbackDataPtr;  /*!< \brief App Data */
}CSR_MESH_APP_EVENT_DATA_T;

/*! \brief CSR Mesh Control Protocol message header format */
typedef struct
{
    CsrUint32 seq;       /*!< \brief Message sequence number      */
    CsrUint16 src;       /*!< \brief Message source address       */
    CsrUint16 dst;       /*!< \brief Message destination address  */
    CsrUint16 opcode;    /*!< \brief Message op-code  */
} CSR_MESH_MCP_MESSAGE_HEADER_T;


/*----------------------------------------------------------------------------*
 * CSR_MESH_MCP_MODEL_HANDLER_T
 */
/*! \brief MCP Model handler function type 
 *
 *
 *  \param nwkId  network id on which the message is received
 * \param pHeader  pHeader of received message 
 *  \param params   params of received message
 *  \param len   length of received message
 *  \param rx_ttl ttl of received message
 *  \param rx_rssi  rssi of received message default value of rssi is zero. 
 *  If message is received on non-LE bearer rssi is zero.for LE bearer the
 *  rssi is expressed in dbm
 *
 *
 *  \returns CSRmeshResult. Refer to \ref CSRmeshResult.
 */
/*---------------------------------------------------------------------------*/
typedef CSRmeshResult (*CSR_MESH_MCP_MODEL_HANDLER_T)(CsrUint8 nwkId,
                                      CSR_MESH_MCP_MESSAGE_HEADER_T *pHeader, 
                                      CsrUint8 *params,
                                      CsrUint8 len,
                                      CsrUint8 rx_ttl,
                                      CsrInt8 rx_rssi);


/*! \brief CSR Mesh Control Protocol MODEL data structure */
typedef struct CSR_MESH_MCP_MODEL
{
    CSR_MESH_MCP_MODEL_HANDLER_T handler; /*!< \brief Model handler function */

    CsrUint8                    number;  /*!< \brief Model number */

    CsrUint8                    netId;  /*!</brief network id for which the model wants to receive 
                                             the data */

    CsrUint16                  *group_ids; /*!< brief Pointer to Group ID list for the
                                        model */

    CsrUint8                   num_group_ids; /*!< brief Number of group IDs supported
                                            by the model */

    struct CSR_MESH_MCP_MODEL  *next_model;  /*!<  \brief Pointer to the next model
                                             data in the supported list of
                                             models */
} CSR_MESH_MCP_MODEL_T;

/*! \brief CSRmesh MCP Model Client Data */
typedef struct CSR_MESH_MCP_MODEL_CLIENT
{
    /*!\brief Model client handler function */
    CSR_MESH_MCP_MODEL_HANDLER_T handler;

    /*!\brief Pointer to the next model client in the list of clients */
    struct CSR_MESH_MCP_MODEL_CLIENT *next_model_client;
} CSR_MESH_MCP_MODEL_CLIENT_T;

/*!\brief CSRmesh Application callback handler function*/
typedef void (*CSR_MESH_APP_CB_T) (CSR_MESH_APP_EVENT_DATA_T eventDataCallback);
/*!\brief CSRmesh Application MASP callback handler function*/
typedef void (*CSR_MESH_APP_MASP_CB_T) (const CsrUint8* message, 
                                        CsrUint8 length,
                                        CsrUint8 rx_ttl,
                                        CsrInt8 rx_rssi);
/*!\brief CSRmesh Application MCP callback handler function*/
typedef void (*CSR_MESH_APP_MCP_CB_T) (CsrUint8 netId, 
                                       const CsrUint8* message, 
                                       CsrUint8 length,
                                       CsrUint8 rx_ttl,
                                       CsrInt8 rx_rssi);

/*!@} */
#endif /* _CSR_MESH_TYPES_H_ */

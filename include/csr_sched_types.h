 /******************************************************************************
 * Copyright 2015 Qualcomm Technologies International, Ltd.
 *****************************************************************************/
/*! \file csr_sched_types.h
 *  \brief CSRmesh Scheduler data types
 *
 *   This file contains the different data types used in CSRmesh Scheduling.
 *
 */
 /*****************************************************************************/

#ifndef __CSR_SCHED_TYPES_H__
#define __CSR_SCHED_TYPES_H__



/*! \addtogroup CSRmesh
 * @{
 */
 #if defined(CSR101x) || defined(CSR101x_A05) 

#include <types.h>
#include <bluetooth.h>
#include <gap_types.h>

/*! \brief Maximum User Advertising data length */
#define MAX_USER_ADV_DATA_LEN                   (31)

/*! \brief CSR Mesh Scheduling Scan Parameter Type */
typedef enum
{
    CSR_SCHED_SCAN_WINDOW_PARAM = 0x00, /*!< \brief scan parameter contains scan window and scan interval */
    CSR_SCHED_SCAN_DUTY_PARAM   = 0x01 /*!< \brief scan parameter contains scan duty cycle and min scan slot */
}CSR_SCHED_SCAN_TYPE_T;

/*! \brief CSR Mesh Scheduling Scan Window Parameter Type */
typedef struct
{
    CsrUint32 scan_window; /*!< \brief scan window param in milliseconds value ranges from 2.5 msecs to 10240 msecs */
    CsrUint32 scan_interval;/*!< \brief scan interval param in milliseconds value ranges 2.5 msecs to 10240 msecs */
}CSR_SCHED_SCAN_WINDOW_PARAM_T;

/*! \brief CSR Mesh Scheduling Scan Duty Parameter Type*/
typedef struct
{
    CsrUint16 scan_duty_cycle; /*!< \brief scan duty cycle 0-100: duty cycle percentage in 1 percent steps for value 101-255
                                 duty cycle = (value - 100)/10 percentage in 0.1 percent steps 0.1 to 15.5% */
    CsrUint16 min_scan_slot; /*!< \brief scan slot value ranges from 0x0004 to 0x4000 */
}CSR_SCHED_SCAN_DUTY_PARAM_T;


typedef union
{
    CSR_SCHED_SCAN_WINDOW_PARAM_T scan_window_param;/*!< \brief user can configure either scan_window  parameter */
    CSR_SCHED_SCAN_DUTY_PARAM_T   scan_duty_param;/*!< \brief user can configure either  scan_duty parameter */
}CSR_SCHED_SCAN_PARAM_T;

/*! \brief CSR Mesh Scheduling Generic LE Parameter Type */
typedef struct
{
    CSR_SCHED_SCAN_TYPE_T scan_param_type;/*!< \brief CSRmesh Scan Parameter type*/
    CSR_SCHED_SCAN_PARAM_T scan_param;/*!< \brief CSRmesh Scan Parameter */
    CsrUint32 advertising_interval; /*!< \brief CSRmesh advertising interval in milliseconds */
    CsrUint32 advertising_time; /*!< \brief CSRmesh advertising time in milliseconds */
    CsrInt8 tx_power;
    ls_addr_type addr_type;
} CSR_SCHED_GENERIC_LE_PARAM_T;

/*! \brief CSR Mesh Scheduling Tx Parameter Type */
typedef struct
{
 CsrUint8 tx_queue_size; /*!< \brief CSRmesh Transmit Queue Size. */
 CsrUint8 relay_repeat_count; /*!< \brief CSRmesh Relay message repeat count. */
 CsrUint8 device_repeat_count; /*!< \brief CSRmesh Device message repeat count. */
} CSR_SCHED_MESH_TX_PARAM_T;

/*! \brief CSR Mesh Scheduling Mesh-LE Parameter Type */
typedef struct
{
    CsrBool is_le_bearer_ready;/*!< \brief Flag to check whether bearer is ready */
    CSR_SCHED_MESH_TX_PARAM_T tx_param;/*!< \brief CSRmesh transmit parameter*/
} CSR_SCHED_MESH_LE_PARAM_T;

/*! \brief CSR Mesh Scheduling LE Parameter Type */
typedef struct
{
 CSR_SCHED_MESH_LE_PARAM_T mesh_le_param;/*!< \brief CSRmesh MESH-LE parameter */
 CSR_SCHED_GENERIC_LE_PARAM_T generic_le_param;/*!< \brief CSRmesh Generic LE parameter */
} CSR_SCHED_LE_PARAMS_T;

/*! \brief CSR Mesh Scheduling LE Advertisment Parameter Type */
typedef struct
{
    TYPED_BD_ADDR_T bd_addr;/*!< \brief CSRmesh BD-ADDR */
    ls_advert_type adv_type;/*!< \brief CSRmesh Advertisment Type*/
    gap_role       role;/*!< \brief CSRmesh Gap Role */
    gap_mode_connect connect_mode;/*!< \brief CSRmesh Gap connect mode */
    gap_mode_discover discover_mode;/*!< \brief CSRmesh Gap discover mode */
    gap_mode_bond bond;/*!< \brief CSRmesh Gap bond mode */
    gap_mode_security security_mode;/*!< \brief CSRmesh Gap security mode */
} CSR_SCHED_ADV_PARAMS_T;

/*! \brief CSR Mesh Scheduling LE Advertising Data  */
typedef struct
{
    CSR_SCHED_ADV_PARAMS_T adv_params;/*!< \ brief User advertising parameters */
    CsrUint8 ad_data[MAX_USER_ADV_DATA_LEN]; /*!< \ brief User advertising data  */
    CsrUint8 ad_data_length;/*!< \ brief advertising data  Length */
    CsrUint8 scan_rsp_data[MAX_USER_ADV_DATA_LEN]; /*!< \brief User Scan response data  */
    CsrUint8 scan_rsp_data_length; /*!< \brief User Scan response data length */
}CSR_SCHED_ADV_DATA_T;

/*! \brief CSR Mesh Scheduling LE Event Type*/
typedef enum
{
    CSR_SCHED_GATT_CONNECTION_EVENT, /*!< \brief GATT Connection event */ 
    CSR_SCHED_GATT_STATE_CHANGE_EVENT,/*!< \brief GATT State change event */ 
    CSR_SCHED_GATT_CCCD_STATE_CHANGE_EVENT/*!< \brief GATT client characteristics configuration descriptor state change event */
}CSR_SCHED_GATT_EVENT_T;

/*! \brief CSR Mesh Scheduling LE Incoming data event Type */
typedef enum
{
    CSR_SCHED_INCOMING_LE_MESH_DATA_EVENT   = 1, /*!< \brief Incoming Mesh data coming from LE ADV event*/ 
    CSR_SCHED_INCOMING_GATT_MESH_DATA_EVENT = 2, /*!< \brief Incoming Mesh data coming from GATT connection event */ 
    CSR_SCHED_SET_LE_ADV_PKT_EVENT          = 3/*!< \brief Incomding Mesh data coming on Gatt Connection and sent over LE ADV event*/
}CSR_SCHED_INCOMING_DATA_EVENT_T;

/*! \brief CSR Mesh Scheduling GATT Event Type */
typedef struct
{
    CsrUint16 cid; /*!< \brief Channel id for which the GATT event is received */
    CsrBool is_gatt_bearer_ready;/*!< \brief Flag to identify the Gatt connection state */
    CsrUint16 conn_interval; /*!< \brief Connection interval value updated during GATT connection. */
    CsrBool  is_notification_enabled;/*!< \brief flag to identify the cccd state */
} CSR_SCHED_GATT_EVENT_DATA_T;

typedef enum
{
    CSR_SCHED_USER_ADV_SENT    = 0x01,
    CSR_SCHED_USER_ADV_PENDING = 0x02
}CSR_SCHED_USER_ADV_STATUS_T;

/*! \brief CSR Mesh Scheduler GATT notify handler function */
typedef void (*CSR_SCHED_NOTIFY_GATT_CB_T) (CsrUint16 ucid, CsrUint8 *mtl_msg, CsrUint8 length);

/*!\brief CSR Mesh Application callback handler function*/
typedef void (*CSR_SCHED_USER_ADV_NOTIFY_CB_T) (CSR_SCHED_USER_ADV_STATUS_T status, CsrUint32 time_us);

/*! \brief CSR Mesh Scheduling operation status */
typedef enum
{
    CSR_SCHED_RESULT_SUCCESS                   = 0x0000,/*!< \brief Operation result success */ 
    CSR_SCHED_RESULT_INVALID_HANDLE            = 0x0001,/*!< \brief Operation result invalid handle */ 
    CSR_SCHED_RESULT_UNACCEPTABLE_ADV_DURATION = 0x0002,/*!< \brief Operation result unacceptable advertisment duration */ 
    CSR_SCHED_RESULT_UNACCEPTABLE_ADV_INTERVAL = 0x0003,/*!< \brief Operation result unacceptable advertisment interval */
    CSR_SCHED_RESULT_INCORRECT_SCAN_PARAM      = 0x0004,/*!< \brief Operation result incorrect san params */
    CSR_SCHED_RESULT_SCAN_NOT_STARTED          = 0x0005,/*!< \brief Operation result scan not started */
    CSR_SCHED_RESULT_FAILURE                   = 0xFFFF/*!< \brief Operation status for failure*/ 
}CSRSchedResult;



/*!@} */

#endif /* defined(CSR101x) || defined(CSR101x_A05) */

#endif /*__CSR_SCHED_TYPES_H__ */



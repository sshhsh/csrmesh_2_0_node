/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  %%version
 *  Application version 2.0
 *
 *  FILE
 *      csr_mesh_bridge.h
 *
 *  DESCRIPTION
 *      Header definitions for CSR Mesh application file
 *
 ******************************************************************************/

#ifndef __CSR_MESH_BRIDGE_H__
#define __CSR_MESH_BRIDGE_H__

 /*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <types.h>
#include <bt_event_types.h>
#include <timer.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"

/*============================================================================*
 *  CSR Mesh Header Files
 *============================================================================*/
#include "csr_mesh.h"
#include "csr_sched.h"
#include "csr_mesh_result.h"

 /*============================================================================*
 *  Public Definitions
 *============================================================================*/
/*! \brief Bluetooth SIG Organization identifier for CSRmesh device appearance */
#define APPEARANCE_ORG_BLUETOOTH_SIG   (0)

/* Maximum number of timers */
#define MAX_APP_TIMERS                 (1 + CSR_MESH_MAX_NO_TIMERS)

/* TGAP(conn_pause_peripheral) defined in Core Specification Addendum 3 Revision
 * 2. A Peripheral device should not perform a Connection Parameter Update proc-
 * -edure within TGAP(conn_pause_peripheral) after establishing a connection.
 */
#define TGAP_CPP_PERIOD                (1 * SECOND)

/* TGAP(conn_pause_central) defined in Core Specification Addendum 3 Revision 2.
 * After the Peripheral device has no further pending actions to perform and the
 * Central device has not initiated any other actions within TGAP(conn_pause_ce-
 * -ntral), then the Peripheral device may perform a Connection Parameter Update
 * procedure.
 */
#define TGAP_CPC_PERIOD                (1 * SECOND)

/* Slave device is not allowed to transmit another Connection Parameter
 * Update request till time TGAP(conn_param_timeout). Refer to section 9.3.9.2,
 * Vol 3, Part C of the Core 4.0 BT spec. The application should retry the
 * 'connection parameter update' procedure after time TGAP(conn_param_timeout)
 * which is 30 seconds.
 */
#define GAP_CONN_PARAM_TIMEOUT         (30 * SECOND)

/* NVM Data Write defer Duration */
#define NVM_WRITE_DEFER_DURATION       (5 * SECOND)

/* Service ID for CSRmesh Adverts is 0xFEF1. */
/* Remove it since it will be present in MTL.h */
#define MTL_ID_CODE                     0xFEF1 /*!< \brief MTL ID code */

/* Association Removal Button Press Duration */
#define LONG_KEYPRESS_TIME             (2 * SECOND)

/* Advertisement Timer for sending device identification */
#define DEVICE_ID_ADVERT_TIME          (5 * SECOND)

/* Magic value to check the sanity of NVM region used by the application */
#define NVM_SANITY_MAGIC               (0xAB90)

/* NVM offset for the application NVM version the app NVM area starts after
 * Mesh Lib NVM
 */
#define NVM_OFFSET_SANITY_WORD         (CSR_MESH_NVM_SIZE)

/* NVM offset for NVM sanity word */
#define NVM_OFFSET_APP_NVM_VERSION     (NVM_OFFSET_SANITY_WORD + 1)

#define NVM_MAX_APP_MEMORY_WORDS       (NVM_OFFSET_APP_NVM_VERSION + 1)

/*============================================================================*
*  Public Data Types
*============================================================================*/

/* CSRmesh device association state */
typedef enum
{

    /* Application Initial State */
    app_state_not_associated = 0,

    /* Application state association started */
    app_state_association_started,

    /* Application state associated */
    app_state_associated,

} app_association_state;

typedef enum
{
    /* Application Initial State */
    app_state_init = 0,

    /* Enters when slow Undirected advertisements are configured */
    app_state_advertising,

    /* Enters when connection is established with the host */
    app_state_connected,

    /* Enters when disconnect is initiated by the application */
    app_state_disconnecting,

    /* Enters when the application is not connected to remote host */
    app_state_idle
} app_state;


typedef struct
{
     /* TYPED_BD_ADDR_T of the host to which device is connected */
    TYPED_BD_ADDR_T                con_bd_addr;

    /* Value for which advertisement timer needs to be started */
    uint32                         advert_timer_value;

    /* Variable to keep track of number of connection
     * parameter update requests made.
     */
    uint8                          num_conn_update_req;

    /* Store timer id for Connection Parameter Update timer in Connected
     * state
     */
    timer_id                       con_param_update_tid;

    /* Connection Parameter Update timer value. Upon a connection, it's started
     * for a period of TGAP_CPP_PERIOD, upon the expiry of which it's restarted
     * for TGAP_CPC_PERIOD. When this timer is running, if a GATT_ACCESS_IND is
     * received, it means, the central device is still doing the service discov-
     * -ery procedure. So, the connection parameter update timer is deleted and
     * recreated. Upon the expiry of this timer, a connection parameter update
     * request is sent to the central device.
     */
    uint32                         cpu_timer_value;

    /* Track the UCID as Clients connect and disconnect */
    uint16                         st_ucid;
     /* Boolean flag indicates whether the device is temporary paired or not */
    bool                           paired;

   /* Variable to store the current connection interval being used. */
    uint16                         conn_interval;
    /* Variable to store the current slave latency. */
    uint16                         conn_latency;
    /* Variable to store the current connection time-out value. */
    uint16                         conn_timeout;
    /* Store timer id for GATT connectable advertisements */
    timer_id                       gatt_advert_tid;
}APP_GATT_SERVICE_DATA_T;

 /* CSRmesh bridge application data structure */
typedef struct
{
    /* Application GATT data */
    APP_GATT_SERVICE_DATA_T        gatt_data;

    ls_addr_type     addr_type;

    /* Current state of application */
    app_state                      state;

    /* Local Device's Random Bluetooth Device Address. */
#ifdef USE_STATIC_RANDOM_ADDRESS
    BD_ADDR_T                      random_bd_addr;
#endif /* USE_STATIC_RANDOM_ADDRESS */

    CSR_SCHED_LE_PARAMS_T          le_params;

    CSR_SCHED_GATT_EVENT_DATA_T    gatt_event_data;

    CSR_MESH_TRANSMIT_STATE_T      bearer_tx_state;

    uint8   netId;

} CSRMESH_BRIDGE_APP_DATA_T;

/*============================================================================*
 *  Public Data
 *============================================================================*/
/* CSR mesh test application specific data */
extern CSRMESH_BRIDGE_APP_DATA_T g_bridgeapp_data;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function is used to set the state of the application */
extern void AppSetState(app_state new_state);

/* Re-initialise application data */
extern void AppDataInit(void);

#endif /* __CSR_MESH_BRIDGE_H__ */


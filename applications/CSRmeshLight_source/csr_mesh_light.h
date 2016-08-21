/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  %%version
 *  Application version 2.0
 *
 *  FILE
 *      csr_mesh_light.h
 *
 *  DESCRIPTION
 *      Header definitions for CSR Mesh application file
 *
 ******************************************************************************/

#ifndef __CSR_MESH_LIGHT_H__
#define __CSR_MESH_LIGHT_H__

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
#include "light_server.h"
#include "power_server.h"
#include "attention_server.h"
#include "battery_server.h"
#include "app_data_stream.h"

 /*============================================================================*
 *  Public Definitions
 *============================================================================*/
/*! \brief Bluetooth SIG Organization identifier for CSRmesh device appearance */
#define APPEARANCE_ORG_BLUETOOTH_SIG   (0)

 #ifdef ENABLE_DEVICE_UUID_ADVERTS
/* Maximum number of timers */
#define MAX_APP_TIMERS                 (8 + CSR_MESH_MAX_NO_TIMERS)
#else
/* Maximum number of timers */
#define MAX_APP_TIMERS                 (7 + CSR_MESH_MAX_NO_TIMERS)
#endif /* ENABLE_DEVICE_UUID_ADVERTS */

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

/* Advertisement Timer for sending device identification */
#define DEVICE_ID_ADVERT_TIME          (5 * SECOND)

/* Default network id of the application */
#define DEFAULT_NW_ID                  (0)

/* The broadcast id for MESH is defined as 0 */
#define MESH_BROADCAST_ID              (0)

/* Magic value to check the sanity of NVM region used by the application */
#define NVM_SANITY_MAGIC               (0xAB90)

/* NVM offset for the application NVM version the app NVM area starts after
 * Mesh Lib NVM
 */
#define NVM_OFFSET_SANITY_WORD         (CSR_MESH_NVM_SIZE)

/* NVM offset for NVM sanity word */
#define NVM_OFFSET_APP_NVM_VERSION     (NVM_OFFSET_SANITY_WORD + 1)

/* Number of words of NVM used by application. Memory used by supported
 * services is not taken into consideration here. */
#define NVM_OFFSET_ASSOCIATION_STATE   (NVM_OFFSET_APP_NVM_VERSION + 1)

#define NVM_OFFSET_BEARER_STATE        (NVM_OFFSET_ASSOCIATION_STATE + 1)

/* NVM Offset for RGB data */
#define NVM_RGB_DATA_OFFSET            (NVM_OFFSET_BEARER_STATE + \
                                           sizeof(CSR_MESH_BEARER_STATE_DATA_T))
/* Size of RGB Data in Words */
#define NVM_RGB_DATA_SIZE              (2)


#define NVM_OFFSET_LIGHT_MODEL_GROUPS  (NVM_RGB_DATA_OFFSET + NVM_RGB_DATA_SIZE)

#define NVM_OFFSET_POWER_MODEL_GROUPS  (NVM_OFFSET_LIGHT_MODEL_GROUPS + \
                                        sizeof(uint16)*MAX_MODEL_GROUPS)

#define NVM_OFFSET_ATT_MODEL_GROUPS    (NVM_OFFSET_POWER_MODEL_GROUPS + \
                                        sizeof(uint16)*MAX_MODEL_GROUPS)

#define NVM_OFFSET_DATA_MODEL_GROUPS   (NVM_OFFSET_ATT_MODEL_GROUPS + \
                                        sizeof(uint16)*MAX_MODEL_GROUPS)

#ifdef ENABLE_DATA_MODEL
#define SIZEOF_DATA_MODEL_GROUPS       (sizeof(uint16)*MAX_MODEL_GROUPS)
#else
#define SIZEOF_DATA_MODEL_GROUPS       (0)
#endif /* ENABLE_DATA_MODEL */

/* NVM Offset for Application data */
#define NVM_MAX_APP_MEMORY_WORDS       (NVM_OFFSET_DATA_MODEL_GROUPS + \
                                        SIZEOF_DATA_MODEL_GROUPS)

/* The User key index where the application config flags are stored */
#define CSKEY_INDEX_USER_FLAGS         (0)

/* Configuration bits on the User Key */
#define CSKEY_RELAY_ENABLE_BIT         (1)
#define CSKEY_BRIDGE_ENABLE_BIT        (2)
#define CSKEY_RANDOM_UUID_ENABLE_BIT   (4)
#define UUID_LENGTH_WORDS              (8)
#define AUTH_CODE_LENGTH_WORDS         (4)

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

 /* CSRmesh Light application data structure */
typedef struct
{
    /* Application GATT data */
    APP_GATT_SERVICE_DATA_T        gatt_data;

    ls_addr_type                   addr_type;

    /* Current state of application */
    app_state                      state;

    /* CSRmesh Association State */
    app_association_state          assoc_state;

    /* Local Device's Random Bluetooth Device Address. */
#ifdef USE_STATIC_RANDOM_ADDRESS
    BD_ADDR_T                      random_bd_addr;
#endif /* USE_STATIC_RANDOM_ADDRESS */

    /* Timer to check for RGB change and store on NVM */
    timer_id                       nvm_tid;

 #ifdef ENABLE_DEVICE_UUID_ADVERTS
    /* CSRmesh transmit device id advertise timer id */
    timer_id                       mesh_device_id_advert_tid;
#endif /* ENABLE_DEVICE_UUID_ADVERTS */

    CSR_SCHED_LE_PARAMS_T          le_params;

    CSR_SCHED_GATT_EVENT_DATA_T    gatt_event_data;

    CSR_MESH_TRANSMIT_STATE_T      bearer_tx_state;

    /* Light model state data */
    CSRMESH_LIGHT_STATE_T          light_model;

    /* Power model state data */
    CSRMESH_POWER_STATE_T          power_model;

    /* Attention model state data */
    CSRMESH_ATTENTION_STATE_T      attn_model;

    /* Battery model state data */
    CSRMESH_BATTERY_STATE_T        battery_model;

    uint8   netId;

} CSRMESH_LIGHT_APP_DATA_T;

/*============================================================================*
*  Public Data
*============================================================================*/
/* Place holders to store assigned model group IDs */
extern uint16 light_model_groups[MAX_MODEL_GROUPS];
extern uint16 power_model_groups[MAX_MODEL_GROUPS];
extern uint16 attention_model_groups[MAX_MODEL_GROUPS];
extern uint16 data_model_groups[MAX_MODEL_GROUPS];

/* CSR mesh test application specific data */
extern CSRMESH_LIGHT_APP_DATA_T g_lightapp_data;

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function is used to set the state of the application */
extern void AppSetState(app_state new_state);

/* Re-initialise application data */
extern void AppDataInit(void);

#endif /* __CSR_MESH_LIGHT_H__ */


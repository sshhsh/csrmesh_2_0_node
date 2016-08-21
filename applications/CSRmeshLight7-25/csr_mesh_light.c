/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  CSR Bluetooth Low Energy CSRmesh test application
 *
 *
 *  FILE
 *      csr_mesh_light.c
 *
 *  DESCRIPTION
 *      This file implements the CSR Mesh application.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <main.h>
#include <ls_app_if.h>
#include <gatt.h>
#include <timer.h>
#include <uart.h>
#include <pio.h>
#include <nvm.h>
#include <security.h>
#include <gatt_prim.h>
#include <mem.h>
#include <panic.h>
#include <config_store.h>
#include <random.h>
#include <buf_utils.h>
#include <debug.h>
/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "app_gatt.h"
#include "app_debug.h"
#include "battery_hw.h"
#include "nvm_access.h"
#include "appearance.h"
#include "gap_service.h"
#include "app_gatt_db.h"
#include "csr_mesh_light.h"
#include "csr_mesh_light_hw.h"
#include "csr_mesh_light_gatt.h"
#include "csr_mesh_light_util.h"
#include "app_mesh_event_handler.h"
#include "app_fw_event_handler.h"
#include "app_data_stream.h"
#include "mesh_control_service_uuids.h"
#include "mesh_control_service.h"
#include "csr_ota.h"
#include "csr_ota_service.h"
#include "gatt_service.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/
#define DEVICE_AUTHCODE_SIZE_IN_BYTES  (8)
#define TX_QUEUE_SIZE                  (8)
#define DEFAULT_SCAN_DUTY_CYCLE        (100)
#define DEFAULT_ADV_INTERVAL           (90 * MILLISECOND)
#define DEFAULT_ADV_TIME               (600 * MILLISECOND)
#define ONE_SHOT_ADV_TIME              (5 * MILLISECOND)
#define DEFAULT_TX_POWER               (7)
#define DEFAULT_ADDR_TYPE              (ls_addr_type_random)
#define DEVICE_REPEAT_COUNT            (DEFAULT_ADV_TIME)/(DEFAULT_ADV_INTERVAL\
                                        + ONE_SHOT_ADV_TIME)
#define RELAY_REPEAT_COUNT             (DEVICE_REPEAT_COUNT / 2)
#define DEFAULT_MIN_SCAN_SLOT          (0x0004)

/*============================================================================*
 *  Private Data
 *===========================================================================*/
/* Declare space for application timers. */
static uint16 app_timers[SIZEOF_APP_TIMER * MAX_APP_TIMERS];

/*============================================================================*
 *  Public Data
 *============================================================================*/
/* CSRmesh light application specific data */
CSRMESH_LIGHT_APP_DATA_T g_lightapp_data;
/* Declare space for Model Groups */
uint16 light_model_groups[MAX_MODEL_GROUPS];
uint16 power_model_groups[MAX_MODEL_GROUPS];
uint16 attention_model_groups[MAX_MODEL_GROUPS];
uint16 data_model_groups[MAX_MODEL_GROUPS];

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* UART Receive callback */
#ifdef DEBUG_ENABLE
static uint16 UartDataRxCallback ( void* p_data, uint16 data_count,
                                   uint16* p_num_additional_words );
#endif /* DEBUG_ENABLE */

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/
/*-----------------------------------------------------------------------------*
 *  NAME
 *      UartDataRxCallback
 *
 *  DESCRIPTION
 *      This callback is issued when data is received over UART. Application
 *      may ignore the data, if not required. For more information refer to
 *      the API documentation for the type "uart_data_out_fn"
 *
 *  RETURNS
 *      The number of words processed, return data_count if all of the received
 *      data had been processed (or if application don't care about the data)
 *
 *----------------------------------------------------------------------------*/
#ifdef DEBUG_ENABLE
static uint16 UartDataRxCallback ( void* p_data, uint16 data_count,
        uint16* p_num_additional_words )
{
    /* Application needs 1 additional data to be received */
    *p_num_additional_words = 1;

    return data_count;
}
#endif /* DEBUG_ENABLE */


/*-----------------------------------------------------------------------------*
 *  NAME
 *      SetMeshConfigParams
 *
 *  DESCRIPTION
 *      This function sets the configuration parameters for csrmesh.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void SetMeshConfigParams(CSR_SCHED_MESH_LE_PARAM_T *mesh_le_params)
{
    mesh_le_params->is_le_bearer_ready           = TRUE;
    mesh_le_params->tx_param.device_repeat_count = DEVICE_REPEAT_COUNT;
    mesh_le_params->tx_param.relay_repeat_count  = RELAY_REPEAT_COUNT;
    mesh_le_params->tx_param.tx_queue_size       = TX_QUEUE_SIZE;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      SetGenericConfigParams
 *
 *  DESCRIPTION
 *      This function sets the generic configuration parameters for csrmesh.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void SetGenericConfigParams(
                            CSR_SCHED_GENERIC_LE_PARAM_T *generic_le_params)
{
    generic_le_params->scan_param_type = CSR_SCHED_SCAN_DUTY_PARAM;
    generic_le_params->scan_param.scan_duty_param.scan_duty_cycle = DEFAULT_SCAN_DUTY_CYCLE;
    generic_le_params->scan_param.scan_duty_param.min_scan_slot   = DEFAULT_MIN_SCAN_SLOT;
    generic_le_params->advertising_interval = DEFAULT_ADV_INTERVAL;
    generic_le_params->advertising_time     = DEFAULT_ADV_TIME;
    generic_le_params->tx_power             = DEFAULT_TX_POWER;
    generic_le_params->addr_type            = DEFAULT_ADDR_TYPE;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      SetLeConfigParams
 *
 *  DESCRIPTION
 *      This function sets the LE configuration parameters for csrmesh.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void SetLeConfigParams(CSR_SCHED_LE_PARAMS_T *le_param)
{
    SetMeshConfigParams(&le_param->mesh_le_param);
    SetGenericConfigParams(&le_param->generic_le_param);
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppDataInit
 *
 *  DESCRIPTION
 *      This function is called to initialise CSRmesh light application
 *      data structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppDataInit(void)
{
    TimerDelete(g_lightapp_data.gatt_data.con_param_update_tid);
    g_lightapp_data.gatt_data.con_param_update_tid = TIMER_INVALID;
    g_lightapp_data.gatt_data.cpu_timer_value = 0;

    g_lightapp_data.gatt_data.st_ucid = GATT_INVALID_UCID;
    g_lightapp_data.gatt_data.advert_timer_value = 0;

    /* Reset the connection parameter variables. */
    g_lightapp_data.gatt_data.conn_interval = 0;
    g_lightapp_data.gatt_data.conn_latency = 0;
    g_lightapp_data.gatt_data.conn_timeout = 0;

    g_lightapp_data.gatt_data.paired = FALSE;
    /* Initialise GAP Data structure */
    GapDataInit();

    /* Initialize the Mesh Control Service Data Structure */
    MeshControlServiceDataInit();

    /* Initialise GATT Data structure */
    GattDataInit();
    /* Initialise the CSR OTA Service Data */
    OtaDataInit();
}


/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppInit
 *
 *  DESCRIPTION
 *      This user application function is called after a power-on reset
 *      (including after a firmware panic), after a wakeup from Hibernate or
 *      Dormant sleep states, or after an HCI Reset has been requested.
 *
 *      The last sleep state is provided to the application in the parameter.
 *
 *      NOTE: In the case of a power-on reset, this function is called
 *      after app_power_on_reset().
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppInit(sleep_state last_sleep_state)
{
    uint16 gatt_db_length = 0;
    uint16 *p_gatt_db_pointer = NULL;

    CSRmeshResult result = CSR_MESH_RESULT_FAILURE;

    /* Initialise the application timers */
    TimerInit(MAX_APP_TIMERS, (void*)app_timers);

    /* Initialise GATT entity */
    GattInit();
    
    /* Install GATT Server support for the optional Write procedure
    * This is mandatory only if control point characteristic is supported.
    */
    GattInstallServerWriteLongReliable();

#ifdef USE_STATIC_RANDOM_ADDRESS
    /* Generate random address for the CSRmesh Device. */
    GenerateStaticRandomAddress(&g_lightapp_data.random_bd_addr);

    /* Set the Static Random Address of the device. */
    GapSetRandomAddress(&g_lightapp_data.random_bd_addr);
#endif /* USE_STATIC_RANDOM_ADDRESS */

#ifdef DEBUG_ENABLE
    /* Initialize UART and configure with
     * default baud rate and port configuration.
     */
    DebugInit(UART_BUF_SIZE_BYTES_256, UartDataRxCallback, NULL);

    /* UART Rx threshold is set to 1,
     * so that every byte received will trigger the rx callback.
     */
    UartRead(1, 0);
#endif /* DEBUG_ENABLE */

    g_lightapp_data.nvm_tid = TIMER_INVALID;
	g_lightapp_data.is_adv = FALSE;

    /* Tell Security Manager module about the value it needs to initialize it's
    * diversifier to.
    */
    SMInit(0);

    /* Intialise application data */
    AppDataInit();
    
    /* Initialise CSRmesh light application State */
    AppSetState(app_state_init);

    /* Set LE Config Params */
    SetLeConfigParams(&g_lightapp_data.le_params);
    CSRSchedSetConfigParams(&g_lightapp_data.le_params);

    /* Initialize Light Hardware */
    LightHardwareInit();
    /* Start ADV GATT Scheduler */
    CSRSchedStart();


    /* Don't wakeup on UART RX line */
    SleepWakeOnUartRX(FALSE);

#ifdef NVM_TYPE_EEPROM
    /* Configure the NVM manager to use I2C EEPROM for NVM store */
    NvmConfigureI2cEeprom();
#elif NVM_TYPE_FLASH
    /* Configure the NVM Manager to use SPI flash for NVM store. */
    NvmConfigureSpiFlash();
#endif /* NVM_TYPE_EEPROM */

    NvmDisable();

    /* Read persistent storage */
    ReadPersistentStore();

    /* Register with CSR Mesh */
#ifdef USE_AUTHORISATION_CODE
    result = CSRmeshInit(CSR_MESH_NON_CONFIG_DEVICE_WITH_AUTH_CODE);
#else
    result = CSRmeshInit(CSR_MESH_NON_CONFIG_DEVICE);
#endif /* USE_AUTHORISATION_CODE */

    CSRmeshRegisterAppCallback(CSRmeshAppProcessMeshEvent);

    if(result == CSR_MESH_RESULT_SUCCESS)
    {
        /* Initialize Light Model */
        LightModelInit(0, light_model_groups, MAX_MODEL_GROUPS,
                                                        AppLightEventHandler);

        /* Initialize the power model */
        PowerModelInit(0, power_model_groups, MAX_MODEL_GROUPS,
                                                        AppPowerEventHandler);

        /* Initialize the attention model */
        AttentionModelInit(0, attention_model_groups, MAX_MODEL_GROUPS,
                                                    AppAttentionEventHandler);

        /* Initialize the battery model */
        BatteryModelInit(0, NULL, MAX_MODEL_GROUPS, AppBatteryEventHandler);

#ifdef ENABLE_DATA_MODEL
        /* Initialize the data model */
        AppDataStreamInit(data_model_groups, MAX_MODEL_GROUPS);
#endif /* ENABLE_DATA_MODEL */

        /* Start CSRmesh */
        result = CSRmeshStart();

        if(result == CSR_MESH_RESULT_SUCCESS)
        {
            /* We do not consider relay priorities for MASP and MCP in this 
             * application so keep the MCP and MASP relay state enabled always.
             * Also enable the transmit state promiscous mode always. The 
             * device promiscuous mode will be controlled using the individual
             * bearer promiscuous setting.
             */
            g_lightapp_data.bearer_tx_state.mcpRelayInfo.relayEnable = TRUE;
            g_lightapp_data.bearer_tx_state.mcpRelayInfo.netId = 
                                                        CSR_MESH_DEFAULT_NETID;
            g_lightapp_data.bearer_tx_state.promiscuousEnable = TRUE;
            g_lightapp_data.bearer_tx_state.maspRelayEnable = TRUE;

            /* Post MASP_DEVICE_ID msg in Bearer TX queue */
            if(g_lightapp_data.assoc_state == app_state_not_associated)
            {
                /* Start sending device UUID adverts */
                InitiateAssociation();
            }
            else
            {
                DEBUG_STR("Light is associated\r\n");
                bool light_poweron = FALSE;

                /* Light is already associated. Set the colour from NVM */
                LightHardwareSetColor(g_lightapp_data.light_model.red,
                g_lightapp_data.light_model.green,
                g_lightapp_data.light_model.blue);

                /* Set the light power as read from NVM */
                if ((g_lightapp_data.power_model.state == csr_mesh_power_state_on) ||
                    (g_lightapp_data.power_model.state ==
                                            csr_mesh_power_state_onfromstandby))
                {
                    light_poweron = TRUE;
                }

                LightHardwarePowerControl(light_poweron);
            }
            /* Update relay and promiscuous settings as per device state */
            AppUpdateBearerState(&g_lightapp_data.bearer_tx_state);
        }
    }
    else
    {
        /* Registration has failed */
    }

    /* Tell GATT about our database. We will get a GATT_ADD_DB_CFM event when
    * this has completed.
    */
    p_gatt_db_pointer = GattGetDatabase(&gatt_db_length);
    GattAddDatabaseReq(gatt_db_length, p_gatt_db_pointer);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppSetState
 *
 *  DESCRIPTION
 *      This function is used to set the state of the application.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void AppSetState(app_state new_state)
{
    /* Check if the new state to be set is not the same as the present state
     * of the application.
     */
    app_state old_state = g_lightapp_data.state;

    if (old_state != new_state)
    {
        /* Handle exiting old state */
        switch (old_state)
        {
            case app_state_disconnecting:
                /* Common things to do whenever application exits
                 * app_state_disconnecting state.
                 */

                /* Initialise CSRmesh light data and services
                 * data structure while exiting Disconnecting state.
                 */
                AppDataInit();
            break;

            case app_state_advertising:
                /* Common things to do whenever application exits
                 * APP_*_ADVERTISING state.
                 */
                /* Stop on-going advertisements */
                GattStopAdverts();
            break;

            case app_state_connected:
                /* Do nothing here */
            break;

            case app_state_idle:
            {
            }
            break;

            default:
                /* Nothing to do */
            break;
        }

        /* Set new state */
        g_lightapp_data.state = new_state;

        /* Handle entering new state */
        switch (new_state)
        {
            case app_state_advertising:
            {
                GattTriggerConnectableAdverts();
            }
            break;

            case app_state_idle:
            {
                /* Resume Scan and Non Connectable Advertisement */

            }
            break;

            case app_state_connected:
            {
                DEBUG_STR("Connected\r\n");
            }
            break;

            case app_state_disconnecting:
                GattDisconnectReq(g_lightapp_data.gatt_data.st_ucid);
            break;

            default:
            break;
        }
    }
}


/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppProcesSystemEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a system event, such
 *      as a battery low notification, is received by the system.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void AppProcessSystemEvent(sys_event_id id, void *data)
{
    switch (id)
    {
        case sys_event_pio_changed:
        {
#ifdef USE_ASSOCIATION_REMOVAL_KEY
             HandlePIOEvent((pio_changed_data*)data);
#endif
        }
        break;

        case sys_event_battery_low:
        {
            /* Battery low event received - Broadcast the battery state to the
             * network.
             */
            SendLowBatteryIndication();
        }
        break;

        default:
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppProcessLmEvent
 *
 *  DESCRIPTION
 *      This user application function is called whenever a LM-specific event
 *      is received by the system.
 *
 * PARAMETERS
 *      event_code [in]   LM event ID
 *      event_data [in]   LM event data
 *
 * RETURNS
 *      TRUE if the application has finished with the event data;
 *           the control layer will free the buffer.
 *----------------------------------------------------------------------------*/
bool AppProcessLmEvent(lm_event_code event_code,
                              LM_EVENT_T *p_event_data)
{
    switch(event_code)
    {
        /* Handle events received from Firmware */

        case GATT_ADD_DB_CFM:
            /* Attribute database registration confirmation */
            HandleSignalGattAddDBCfm((GATT_ADD_DB_CFM_T*)p_event_data);
        break;

        case GATT_CANCEL_CONNECT_CFM:
            /* Confirmation for the completion of GattCancelConnectReq()
             * procedure
             */
            HandleSignalGattCancelConnectCfm(
                                    (GATT_CANCEL_CONNECT_CFM_T*)p_event_data);
        break;

        case LM_EV_CONNECTION_COMPLETE:
            /* Handle the LM connection complete event. */
            HandleSignalLmEvConnectionComplete((LM_EV_CONNECTION_COMPLETE_T*)
                                                                p_event_data);
        break;

        case GATT_CONNECT_CFM:
            /* Confirmation for the completion of GattConnectReq()
             * procedure
             */
            HandleSignalGattConnectCfm((GATT_CONNECT_CFM_T*)p_event_data);
        break;

        case SM_SIMPLE_PAIRING_COMPLETE_IND:
            /* Indication for completion of Pairing procedure */
            HandleSignalSmSimplePairingCompleteInd(
                (SM_SIMPLE_PAIRING_COMPLETE_IND_T*)p_event_data);
        break;

        case LM_EV_ENCRYPTION_CHANGE:
            /* Indication for encryption change event */
            /* Nothing to do */
        break;

        /* Received in response to the LsConnectionParamUpdateReq()
         * request sent from the slave after encryption is enabled. If
         * the request has failed, the device should again send the same
         * request only after Tgap(conn_param_timeout). Refer Bluetooth 4.0
         * spec Vol 3 Part C, Section 9.3.9 and HID over GATT profile spec
         * section 5.1.2.
         */
        case LS_CONNECTION_PARAM_UPDATE_CFM:
            HandleSignalLsConnParamUpdateCfm(
                (LS_CONNECTION_PARAM_UPDATE_CFM_T*) p_event_data);
        break;

        case LM_EV_CONNECTION_UPDATE:
            /* This event is sent by the controller on connection parameter
             * update.
             */
            HandleSignalLmConnectionUpdate(
                            (LM_EV_CONNECTION_UPDATE_T*)p_event_data);
        break;

        case LS_CONNECTION_PARAM_UPDATE_IND:
            /* Indicates completion of remotely triggered Connection
             * parameter update procedure
             */
            HandleSignalLsConnParamUpdateInd(
                            (LS_CONNECTION_PARAM_UPDATE_IND_T *)p_event_data);
        break;

        case GATT_ACCESS_IND:
            /* Indicates that an attribute controlled directly by the
             * application (ATT_ATTR_IRQ attribute flag is set) is being
             * read from or written to.
             */
            HandleSignalGattAccessInd((GATT_ACCESS_IND_T*)p_event_data);
        break;

        case GATT_DISCONNECT_IND:
            /* Disconnect procedure triggered by remote host or due to
             * link loss is considered complete on reception of
             * LM_EV_DISCONNECT_COMPLETE event. So, it gets handled on
             * reception of LM_EV_DISCONNECT_COMPLETE event.
             */
         break;

        case GATT_DISCONNECT_CFM:
            /* Confirmation for the completion of GattDisconnectReq()
             * procedure is ignored as the procedure is considered complete
             * on reception of LM_EV_DISCONNECT_COMPLETE event. So, it gets
             * handled on reception of LM_EV_DISCONNECT_COMPLETE event.
             */
        break;

        case LM_EV_DISCONNECT_COMPLETE:
        {
            /* Disconnect procedures either triggered by application or remote
             * host or link loss case are considered completed on reception
             * of LM_EV_DISCONNECT_COMPLETE event
             */
             HandleSignalLmDisconnectComplete(
                    &((LM_EV_DISCONNECT_COMPLETE_T *)p_event_data)->data);
        }
        break;

        case LM_EV_ADVERTISING_REPORT:
        {
            HandleLEAdvMessage((LM_EV_ADVERTISING_REPORT_T *)p_event_data);
        }
        break;

        case LS_RADIO_EVENT_IND:
        {
            CSRSchedNotifyGattEvent(CSR_SCHED_GATT_CONNECTION_EVENT,NULL,NULL);
        }
        break;

        default:
            /* Ignore any other event */
        break;

    }

    return TRUE;
}


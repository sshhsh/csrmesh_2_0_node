/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      app_fw_event_handler.c
 *
 *
 ******************************************************************************/
 /*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <gatt.h>
#include <timer.h>
#include <security.h>
#include <gatt_prim.h>
#include <mem.h>
#include <hci_event_types.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"
#include "app_debug.h"
#include "app_gatt.h"
#include "gap_service.h"
#include "app_gatt_db.h"
#include "csr_mesh_heater.h"
#include "csr_mesh_heater_gatt.h"
#include "csr_mesh_heater_util.h"
#include "app_fw_event_handler.h"

#include "csr_ota.h"
#include "csr_ota_service.h"
#include "gatt_service.h"

/*============================================================================*
 *  Private Data
 *============================================================================*/
/* Buffer to store unpacked data */
static uint8  unpackedData[MAX_ADV_DATA_LEN];

/* TTL of received message */
static uint8 rx_ttl;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
/* Connection parameter update timer handler */
static void requestConnParamUpdate(timer_id tid);
static void handleGapCppTimerExpiry(timer_id tid);

/*============================================================================*
 *  Private Function Definitions
 *============================================================================*/
/*-----------------------------------------------------------------------------*
 *  NAME
 *      handleGapCppTimerExpiry
 *
 *  DESCRIPTION
 *      This function handles the expiry of TGAP(conn_pause_peripheral) timer.
 *      It starts the TGAP(conn_pause_central) timer, during which, if no activ-
 *      -ity is detected from the central device, a connection parameter update
 *      request is sent.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void handleGapCppTimerExpiry(timer_id tid)
{
    if(g_heater_app_data.gatt_data.con_param_update_tid == tid)
    {
        g_heater_app_data.gatt_data.con_param_update_tid =
                     TimerCreate(TGAP_CPC_PERIOD, TRUE, requestConnParamUpdate);
        g_heater_app_data.gatt_data.cpu_timer_value = TGAP_CPC_PERIOD; 
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      requestConnParamUpdate
 *
 *  DESCRIPTION
 *      This function is used to send L2CAP_CONNECTION_PARAMETER_UPDATE_REQUEST
 *      to the remote device when an earlier sent request had failed.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void requestConnParamUpdate(timer_id tid)
{
    /* Application specific preferred parameters */
    ble_con_params app_pref_conn_param;

    if(g_heater_app_data.gatt_data.con_param_update_tid == tid)
    {
        g_heater_app_data.gatt_data.con_param_update_tid = TIMER_INVALID;
        g_heater_app_data.gatt_data.cpu_timer_value = 0;

        /*Handling signal as per current state */
        switch(g_heater_app_data.state)
        {

            case app_state_connected:
            {
                /* Increment the count for Connection Parameter Update
                 * requests
                 */
                ++ g_heater_app_data.gatt_data.num_conn_update_req;

                /* If it is first or second request, preferred connection
                 * parameters should be request
                 */
                if(g_heater_app_data.gatt_data.num_conn_update_req == 1 ||
                   g_heater_app_data.gatt_data.num_conn_update_req == 2)
                {
                    app_pref_conn_param.con_max_interval =
                                                PREFERRED_MAX_CON_INTERVAL;
                    app_pref_conn_param.con_min_interval =
                                                PREFERRED_MIN_CON_INTERVAL;
                    app_pref_conn_param.con_slave_latency =
                                                PREFERRED_SLAVE_LATENCY;
                    app_pref_conn_param.con_super_timeout =
                                                PREFERRED_SUPERVISION_TIMEOUT;
                }
                /* If it is 3rd or 4th request, APPLE compliant parameters
                 * should be requested.
                 */
                else if(g_heater_app_data.gatt_data.num_conn_update_req == 3 ||
                        g_heater_app_data.gatt_data.num_conn_update_req == 4)
                {
                    app_pref_conn_param.con_max_interval =
                                                APPLE_MAX_CON_INTERVAL;
                    app_pref_conn_param.con_min_interval =
                                                APPLE_MIN_CON_INTERVAL;
                    app_pref_conn_param.con_slave_latency =
                                                APPLE_SLAVE_LATENCY;
                    app_pref_conn_param.con_super_timeout =
                                                APPLE_SUPERVISION_TIMEOUT;
                }

                /* Send Connection Parameter Update request using application
                 * specific preferred connection parameters
                 */

                if(LsConnectionParamUpdateReq(
                                 &g_heater_app_data.gatt_data.con_bd_addr,
                                 &app_pref_conn_param) != ls_err_none)
                {
                    ReportPanic(app_panic_con_param_update);
                }
            }
            break;

            default:
                /* Ignore in other states */
            break;
        }

    } /* Else ignore the timer */

}

/*============================================================================*
 *  Public Function Definitions
 *============================================================================*/
/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleSignalGattAddDBCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_ADD_DB_CFM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandleSignalGattAddDBCfm(GATT_ADD_DB_CFM_T *p_event_data)
{
    DEBUG_STR("\r\nINFO: in handleSignalGattAddDBCfm\r\n");
    
    switch(g_heater_app_data.state)
    {
        case app_state_init:
        {
            if(p_event_data->result == sys_status_success)
            {
                /* Always do slow adverts on GATT Connection */
                AppSetState(app_state_advertising);
            }
            else
            {
                /* Don't expect this to happen */
                ReportPanic(app_panic_db_registration);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleSignalGattCancelConnectCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CANCEL_CONNECT_CFM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandleSignalGattCancelConnectCfm(GATT_CANCEL_CONNECT_CFM_T
                                             *p_event_data)
{
    if(p_event_data->result != sys_status_success)
    {
        return;
    }
    /*Handling signal as per current state */
    switch(g_heater_app_data.state)
    {
        case app_state_advertising:
            /* There is no idle state, the device
             * will advertise for ever
             */
        break;

        case app_state_connected:
            /* The CSRmesh could have been sending data on
             * advertisements so do not panic
             */
        break;

        case app_state_idle:
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*---------------------------------------------------------------------------
 *
 *  NAME
 *      HandleSignalLmEvConnectionComplete
 *
 *  DESCRIPTION
 *      This function handles the signal LM_EV_CONNECTION_COMPLETE.
 *
 *  RETURNS
 *      Nothing.
 *

*----------------------------------------------------------------------------*/
extern void HandleSignalLmEvConnectionComplete(
                                     LM_EV_CONNECTION_COMPLETE_T *p_event_data)
{
    /* Store the connection parameters. */
    g_heater_app_data.gatt_data.conn_interval = 
                                        p_event_data->data.conn_interval;
    g_heater_app_data.gatt_data.conn_latency  = 
                                        p_event_data->data.conn_latency;
    g_heater_app_data.gatt_data.conn_timeout  =
                                        p_event_data->data.supervision_timeout;
    g_heater_app_data.gatt_event_data.conn_interval = 
                                        p_event_data->data.conn_interval;
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleSignalGattConnectCfm
 *
 *  DESCRIPTION
 *      This function handles the signal GATT_CONNECT_CFM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandleSignalGattConnectCfm(GATT_CONNECT_CFM_T* p_event_data)
{
    CSR_MESH_TRANSMIT_STATE_T bearer_tx_state;

    DEBUG_STR("\r\nINFO: in HandleSignalGattConnectCfm\r\n");
    
    /*Handling signal as per current state */
    switch(g_heater_app_data.state)
    {
        case app_state_advertising:
        {
            if(p_event_data->result == sys_status_success)
            {
                /* Store received UCID */
                g_heater_app_data.gatt_data.st_ucid = p_event_data->cid;

                /* Store connected BD Address */
                g_heater_app_data.gatt_data.con_bd_addr = p_event_data->bd_addr;

                /* Enter connected state */
                AppSetState(app_state_connected);

                /* Update the  GATT Event data attributes */
                g_heater_app_data.gatt_event_data.conn_interval = 
                                                            p_event_data->cid;
                g_heater_app_data.gatt_event_data.is_gatt_bearer_ready = TRUE;

                /* Inform CSRmesh that we are connected now */
                CSRSchedNotifyGattEvent(CSR_SCHED_GATT_STATE_CHANGE_EVENT, 
                                      &g_heater_app_data.gatt_event_data, NULL);

                /* Enable the radio event that indicates the beginning of a 
                 * connection packet transmit during connection event.
                 */
                LsRadioEventNotification(g_heater_app_data.gatt_data.st_ucid, 
                                                  radio_event_connection_event);

                /* The device may not have the network key used by the host
                 * when connected as a bridge. If so data received on the 
                 * mesh control service.will not be forwarded on mesh.
                 * Enable promiscuous mode on both bearers temporarily
                 * till the connection exists.
                 */
                bearer_tx_state = g_heater_app_data.bearer_tx_state;
                bearer_tx_state.bearerPromiscuous = 
                                   LE_BEARER_ACTIVE | GATT_SERVER_BEARER_ACTIVE;

                AppUpdateBearerState(&bearer_tx_state);

                /* Since CSRmesh application does not mandate encryption
                 * requirement on its characteristics, the remote master may
                 * or may not encrypt the link. Start a timer  here to give
                 * remote master some time to encrypt the link and on expiry
                 * of that timer, send a connection parameter update request
                 * to remote side.
                 */

                /* Don't request security as this causes connection issues
                 * with Android 4.4
                 *
                 * SMRequestSecurityLevel(&g_heater_app_data.gatt_data.con_bd_addr);
                 */

                /* If the current connection parameters being used don't
                 * comply with the application's preferred connection
                 * parameters and the timer is not running and, start timer
                 * to trigger Connection Parameter Update procedure
                 */
                if((g_heater_app_data.gatt_data.con_param_update_tid ==
                                                        TIMER_INVALID) &&
                   (g_heater_app_data.gatt_data.conn_interval <
                                             PREFERRED_MIN_CON_INTERVAL ||
                    g_heater_app_data.gatt_data.conn_interval >
                                             PREFERRED_MAX_CON_INTERVAL
#if PREFERRED_SLAVE_LATENCY
                    || g_heater_app_data.gatt_data.conn_latency <
                                             PREFERRED_SLAVE_LATENCY
#endif
                   )
                  )
                {
                    /* Set the num of conn update attempts to zero */
                    g_heater_app_data.gatt_data.num_conn_update_req = 0;

                    /* The application first starts a timer of
                     * TGAP_CPP_PERIOD. During this time, the application
                     * waits for the peer device to do the database
                     * discovery procedure. After expiry of this timer, the
                     * application starts one more timer of period
                     * TGAP_CPC_PERIOD. If the application receives any
                     * GATT_ACCESS_IND during this time, it assumes that
                     * the peer device is still doing device database
                     * discovery procedure or some other configuration and
                     * it should not update the parameters, so it restarts
                     * the TGAP_CPC_PERIOD timer. If this timer expires, the
                     * application assumes that database discovery procedure
                     * is complete and it initiates the connection parameter
                     * update procedure.
                     */
                    g_heater_app_data.gatt_data.con_param_update_tid =
                                      TimerCreate(TGAP_CPP_PERIOD, TRUE,
                                                  handleGapCppTimerExpiry);
                    g_heater_app_data.gatt_data.cpu_timer_value =
                                                        TGAP_CPP_PERIOD;
                }
                  /* Else at the expiry of timer Connection parameter
                   * update procedure will get triggered
                   */
            }
            else
            {
                /* We don't use slow advertising. So, switch device
                 * to fast adverts.
                 */
                AppSetState(app_state_advertising);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleSignalSmSimplePairingCompleteInd
 *
 *  DESCRIPTION
 *      This function handles the signal SM_SIMPLE_PAIRING_COMPLETE_IND
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandleSignalSmSimplePairingCompleteInd(
                                 SM_SIMPLE_PAIRING_COMPLETE_IND_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_heater_app_data.state)
    {
        case app_state_connected:
        {
            if(p_event_data->status == sys_status_success)
            {
                /* Store temporary pairing info. */
                g_heater_app_data.gatt_data.paired = TRUE;
            }
            else
            {
                /* Pairing has failed.disconnect the link.*/
                AppSetState(app_state_disconnecting);
            }
        }
        break;

        default:
            /* Firmware may send this signal after disconnection. So don't
             * panic but ignore this signal.
             */
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleSignalLsConnParamUpdateCfm
 *
 *  DESCRIPTION
 *      This function handles the signal LS_CONNECTION_PARAM_UPDATE_CFM.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandleSignalLsConnParamUpdateCfm(
                            LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_heater_app_data.state)
    {
        case app_state_connected:
        {
            /* Received in response to the L2CAP_CONNECTION_PARAMETER_UPDATE
              * request sent from the slave after encryption is enabled. If
              * the request has failed, the device should again send the same
              * request only after Tgap(conn_param_timeout). Refer
              * Bluetooth 4.0 spec Vol 3 Part C, Section 9.3.9 and profile spec.
              */
            if ((p_event_data->status != ls_err_none) &&
                (g_heater_app_data.gatt_data.num_conn_update_req <
                                    MAX_NUM_CONN_PARAM_UPDATE_REQS))
            {
                /* Delete timer if running */
                TimerDelete(g_heater_app_data.gatt_data.con_param_update_tid);

                g_heater_app_data.gatt_data.con_param_update_tid =
                                 TimerCreate(GAP_CONN_PARAM_TIMEOUT,
                                             TRUE, requestConnParamUpdate);
                g_heater_app_data.gatt_data.cpu_timer_value =
                                             GAP_CONN_PARAM_TIMEOUT; 
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleSignalLmConnectionUpdate
 *
 *  DESCRIPTION
 *      This function handles the signal LM_EV_CONNECTION_UPDATE.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandleSignalLmConnectionUpdate(LM_EV_CONNECTION_UPDATE_T* 
                                                                p_event_data)
{

    switch(g_heater_app_data.state)
    {
        case app_state_connected:
        case app_state_disconnecting:
        {
            /* Store the new connection parameters. */
            g_heater_app_data.gatt_data.conn_interval =
                                            p_event_data->data.conn_interval;
            g_heater_app_data.gatt_data.conn_latency =
                                            p_event_data->data.conn_latency;
            g_heater_app_data.gatt_data.conn_timeout =
                                        p_event_data->data.supervision_timeout;
            g_heater_app_data.gatt_event_data.conn_interval =
                                            p_event_data->data.conn_interval;

            CSRSchedNotifyGattEvent(CSR_SCHED_GATT_STATE_CHANGE_EVENT,
                                    &g_heater_app_data.gatt_event_data, NULL);

            /* Enable the radio event that indicates the beginning of a 
             * connection packet transmit during connection event.
             */
            LsRadioEventNotification(g_heater_app_data.gatt_data.st_ucid,
                                                radio_event_connection_event);
        }
        break;

        default:
            /* Connection parameter update indication received in unexpected
             * application state.
             */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleSignalLsConnParamUpdateInd
 *
 *  DESCRIPTION
 *      This function handles the signal LS_CONNECTION_PARAM_UPDATE_IND.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandleSignalLsConnParamUpdateInd(
                                 LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data)
{
    /*Handling signal as per current state */
    switch(g_heater_app_data.state)
    {
        case app_state_connected:
        {
            /* Delete timer if running */
            TimerDelete(g_heater_app_data.gatt_data.con_param_update_tid);
            g_heater_app_data.gatt_data.con_param_update_tid = TIMER_INVALID;
            g_heater_app_data.gatt_data.cpu_timer_value = 0;

            /* The application had already received the new connection
             * parameters while handling event LM_EV_CONNECTION_UPDATE.
             * Check if new parameters comply with application preferred
             * parameters. If not, application shall trigger Connection
             * parameter update procedure
             */

            if(g_heater_app_data.gatt_data.conn_interval <
                                                PREFERRED_MIN_CON_INTERVAL ||
               g_heater_app_data.gatt_data.conn_interval >
                                                PREFERRED_MAX_CON_INTERVAL
#if PREFERRED_SLAVE_LATENCY
               || g_heater_app_data.gatt_data.conn_latency <
                                                PREFERRED_SLAVE_LATENCY
#endif
              )
            {
                /* Set the num of conn update attempts to zero */
                g_heater_app_data.gatt_data.num_conn_update_req = 0;

                /* Start timer to trigger Connection Parameter Update
                 * procedure
                 */
                g_heater_app_data.gatt_data.con_param_update_tid =
                                TimerCreate(GAP_CONN_PARAM_TIMEOUT,
                                            TRUE, requestConnParamUpdate);
                g_heater_app_data.gatt_data.cpu_timer_value =
                                                        GAP_CONN_PARAM_TIMEOUT;
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleSignalGattAccessInd
 *
 *  DESCRIPTION
 *      This function handles GATT_ACCESS_IND message for attributes
 *      maintained by the application.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data)
{

    /*Handling signal as per current state */
    switch(g_heater_app_data.state)
    {
        case app_state_connected:
        {
            /* GATT_ACCESS_IND indicates that the central device is still disco-
             * -vering services. So, restart the connection parameter update
             * timer
             */
             if(g_heater_app_data.gatt_data.cpu_timer_value == TGAP_CPC_PERIOD &&
                g_heater_app_data.gatt_data.con_param_update_tid != TIMER_INVALID)
             {
                TimerDelete(g_heater_app_data.gatt_data.con_param_update_tid);
                g_heater_app_data.gatt_data.con_param_update_tid =
                                    TimerCreate(TGAP_CPC_PERIOD,
                                                TRUE, requestConnParamUpdate);
             }

            /* Received GATT ACCESS IND with write access */
            if(p_event_data->flags & ATT_ACCESS_WRITE)
            {
                /* If only ATT_ACCESS_PERMISSION flag is enabled, then the
                 * firmware is asking the app for permission to go along with
                 * prepare write request from the peer. Allow it.
                 */
                if(((p_event_data->flags) &
                   (ATT_ACCESS_PERMISSION | ATT_ACCESS_WRITE_COMPLETE))
                                                    == ATT_ACCESS_PERMISSION)
                {
                    GattAccessRsp(p_event_data->cid, p_event_data->handle,
                                  sys_status_success, 0, NULL);
                }
                else
                {
                    HandleAccessWrite(p_event_data);
                }
            }
            else if(p_event_data->flags & ATT_ACCESS_WRITE_COMPLETE)
            {
                GattAccessRsp(p_event_data->cid, p_event_data->handle,
                                          sys_status_success, 0, NULL);
            }
            /* Received GATT ACCESS IND with read access */
            else if(p_event_data->flags ==
                                    (ATT_ACCESS_READ | ATT_ACCESS_PERMISSION))
            {
                HandleAccessRead(p_event_data);
            }
            else
            {
                GattAccessRsp(p_event_data->cid, p_event_data->handle,
                              gatt_status_request_not_supported,
                              0, NULL);
            }
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleSignalLmDisconnectComplete
 *
 *  DESCRIPTION
 *      This function handles LM Disconnect Complete event which is received
 *      at the completion of disconnect procedure triggered either by the
 *      device or remote host or because of link loss
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void HandleSignalLmDisconnectComplete(
                HCI_EV_DATA_DISCONNECT_COMPLETE_T *p_event_data)
{

    /* Reset the connection parameter variables. */
    g_heater_app_data.gatt_data.conn_interval = 0;
    g_heater_app_data.gatt_data.conn_latency = 0;
    g_heater_app_data.gatt_data.conn_timeout = 0;

    CSRSchedNotifyGattEvent(CSR_SCHED_GATT_STATE_CHANGE_EVENT, NULL, NULL);

   /* Disable radio event notifications */
   LsRadioEventNotification(GATT_INVALID_UCID, radio_event_none);

    /* Restore bearer promiscuous state */
    AppUpdateBearerState(&g_heater_app_data.bearer_tx_state);

    if(OtaResetRequired())
    {
        OtaReset();
    }

    /*Handling signal as per current state */
    switch(g_heater_app_data.state)
    {
        case app_state_connected:
        case app_state_disconnecting:
        {
            /* Connection is terminated either due to Link Loss or
             * the local host terminated connection. In either case
             * Initialise the app data and go to fast advertising.
             */
            AppDataInit();
            AppSetState(app_state_advertising);
        }
        break;

        default:
            /* Control should never come here */
            ReportPanic(app_panic_invalid_state);
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleLEAdvMessage
 *
 *  DESCRIPTION
 *      Process a received message from the radio
 *
 * PARAMETERS
 *      None
 *
 * RETURNS
 *      Nothing
 *----------------------------------------------------------------------------*/
extern bool HandleLEAdvMessage(LM_EV_ADVERTISING_REPORT_T* report)
{

    bool result = FALSE;
    uint16 *advertData;
    uint8 length;
    uint8 index = 0;

    HCI_EV_DATA_ULP_ADVERTISING_REPORT_T *data = &(report->data);

    /* handle non connectable adv */
    if( data->event_type == ls_advert_non_connectable )
    {
        /* The advert data is supplied to us as a packed uint8 so unpack it
         * locally to allow easier analysis (to find the device name).
         *
         * Get a pointer to the actual data, which is after the control
         * information block and RSSI parameter (the last uint8).
         */
        advertData = (uint16*) data +
                     sizeof(HCI_EV_DATA_ULP_ADVERTISING_REPORT_T) +
                     sizeof(uint8);

        MemCopyUnPack(unpackedData, advertData, data->length_data);

        /* Loop until we find a block with manufacturer specific data */
        do
        {
            if(index + 4 < data->length_data)
            {
                /* Search for CSRmesh UUID data 
                 * Adv packets are of the form:
                 *  |L|T|D....|L|T|D....|...
                 *  L - Length  1 octet. Includes length of T(Type) + D(data)
                 *  T - AD type 1 octet
                 *  D - Data    L minus 1 octets
                 *  For mesh packets, data will be 16 bits UUID followed by mesh
                 *  payload
                 *  Mesh payload size = L minus 3 .
                 */
                if (!MemCmp(&unpackedData[index + 1], mesh_ad_data, 3))
                {
                    /* derive length first */
                    length = unpackedData[index];

                    if(length > MAX_ADV_DATA_LEN || length == 0)
                    {
                        /* We don't have enough data to process */
                        break;
                    }

                    /* Fill in TTL variables */
                    rx_ttl  = unpackedData[index + length];

                    /* Update Bearer Event Data structure with incoming Mesh 
                     * Data.
                     */
                    CSRSchedHandleIncomingData(
                                        CSR_SCHED_INCOMING_LE_MESH_DATA_EVENT, 
                                        &unpackedData[index+4], (length-3), 
                                        report->rssi);
                    result = TRUE;
                    break;
                }
                else
                {
                    /* Not a mesh data */
                }
                index += 1;
            }
            else
            {
                /* We don't have enough data to process */
                break;
            }
        }while(index < data->length_data);
    }
    else
    {
         /* condition to handle connectable undirected adv, connectable
          * directed adv & scan response
          */
    }

    return result;
}

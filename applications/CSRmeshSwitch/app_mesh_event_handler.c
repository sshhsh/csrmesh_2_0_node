/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      app_mesh_event_handler.c
 *
 *
 ******************************************************************************/
 /*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <timer.h>
#include <mem.h>
#include <buf_utils.h>
#include <config_store.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "iot_hw.h"
#include "appearance.h"
#include "user_config.h"
#include "nvm_access.h"
#include "app_debug.h"
#include "app_gatt.h"
#include "gap_service.h"
#include "app_gatt_db.h"
#include "csr_mesh_switch.h"
#include "csr_mesh_switch_gatt.h"
#include "csr_mesh_switch_util.h"
#include "csr_mesh_switch_hw.h"
#include "app_mesh_event_handler.h"
#include "battery_hw.h"
#include "csr_ota.h"
#include "csr_ota_service.h"
#include "gatt_service.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* Application VID,PID and Version. */
CSR_MESH_VID_PID_VERSION_T vid_pid_info =
{
    .vendor_id  = APP_VENDOR_ID,
    .product_id = APP_PRODUCT_ID,
    .version    = APP_VERSION,
};

/* Attention timer id */
static timer_id attn_tid = TIMER_INVALID;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/*============================================================================*
 *  Private Function Definitions
 *============================================================================*/

/*-----------------------------------------------------------------------------*
 *  NAME
 *      attnTimerHandler
 *
 *  DESCRIPTION
 *      This function handles Attention time-out.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void attnTimerHandler(timer_id tid)
{
    if (attn_tid == tid)
    {
        attn_tid = TIMER_INVALID;

        if(g_switchapp_data.assoc_state == app_state_associated)
        {
            /* Stop blink */
            IOTLightControlDevicePower(FALSE);
        }
        else if(g_switchapp_data.assoc_state == app_state_association_started)
        {
            /* Blink Light in Yellow to indicate association in progress */
            IOTLightControlDeviceBlink(127, 127, 0, 32, 32);
        }
        else
        {
            /* Restart Blue blink to indicate ready for association */
            IOTLightControlDeviceBlink(0, 0, 127, 32, 32);
        }
    }
}


/*---------------------------------------------------------------------------
 *
 *  NAME
 *      EnableHighDutyScanMode
 *
 *  DESCRIPTION
 *      The function enables/disables the active scan mode 
 *
 *  RETURNS/MODIFIES
 *      None
 *
 
*----------------------------------------------------------------------------*/
extern void EnableHighDutyScanMode(bool enable)
{
    uint8 scan_duty_cycle;

    if(enable)
    {
        scan_duty_cycle = HIGH_RX_DUTY_CYCLE;
    }
    else
    {
        if(g_switchapp_data.assoc_state == app_state_associated)
        {
            /* Change the Rx scan duty cycle to default val if the device is
             * already associated.
             */
            scan_duty_cycle = DEFAULT_RX_DUTY_CYCLE;
        }
        else
        {
            /* Change the Rx scan duty cycle to active as the device is not
             * associated yet.
             */
            scan_duty_cycle = HIGH_RX_DUTY_CYCLE;
        }
    }
    SetScanDutyCycle(scan_duty_cycle);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      CSRmeshAppProcessMeshEvent
 *
 *  DESCRIPTION
 *   The CSRmesh™ stack calls this call-back to notify asynchronous 
 *   events to applications.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
void CSRmeshAppProcessMeshEvent(CSR_MESH_APP_EVENT_DATA_T eventDataCallback)
{

    CSR_MESH_OPERATION_STATUS_T operation_status = eventDataCallback.status;
    CSR_MESH_EVENT_T             operation_event = eventDataCallback.event;

    /*Handling signal as per current state */
    switch(operation_status)
    {
        case CSR_MESH_OPERATION_SUCCESS:
        {
            switch(operation_event)
            {
                case CSR_MESH_ASSOC_STARTED_EVENT:
                {
                    //DEBUG_STR("\r\n Ref APP: CSR_MESH_ASSOC_STARTED_EVENT\r\n");
                    g_switchapp_data.assoc_state = app_state_association_started;

                    /* Delete the attention timeout timer if enabled */
                    if( attn_tid != TIMER_INVALID)
                    {
                        TimerDelete(attn_tid);
                        attn_tid = TIMER_INVALID;
                    }

                    /* Blink Light in Yellow to indicate association started */
                    LightHardwareSetBlink(127, 127, 0, 32, 32);

                }
                break;
                case CSR_MESH_ASSOC_COMPLETE_EVENT:
                case CSR_MESH_SEND_ASSOC_COMPLETE_EVENT:
                {
                    DEBUG_STR("\r\n Ref APP: CSR_MESH ASSOC_COMPLETE \r\n");
                    g_switchapp_data.assoc_state = app_state_associated;

                    /* Set LED colour to OFF after association is successful */
                    LightHardwareSetColor (0, 0, 0);

                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_switchapp_data.assoc_state,
                             sizeof(g_switchapp_data.assoc_state),
                             NVM_OFFSET_ASSOCIATION_STATE);

                    /* Disable promiscuous mode */
                    g_switchapp_data.bearer_tx_state.bearerPromiscuous = 0;

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_switchapp_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    /* If the device is connected as a bridge, the stored 
                     * promiscuous settings would be assigned at the time of 
                     * disconnection.
                     */
                    if(g_switchapp_data.gatt_data.st_ucid == GATT_INVALID_UCID)
                    {
                        AppUpdateBearerState(&g_switchapp_data.bearer_tx_state);
                    }

#ifdef ENABLE_WATCHDOG_MODEL
                    AppWatchdogStart();
#endif
                }
                break;

                case CSR_MESH_CONFIG_RESET_DEVICE_EVENT:
                {
                    g_switchapp_data.assoc_state = app_state_not_associated;

                    /* Enable promiscuous mode */
                    g_switchapp_data.bearer_tx_state.bearerPromiscuous = 
                                LE_BEARER_ACTIVE | GATT_SERVER_BEARER_ACTIVE;
                    AppUpdateBearerState(&g_switchapp_data.bearer_tx_state);

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_switchapp_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_switchapp_data.assoc_state,
                             sizeof(g_switchapp_data.assoc_state),
                             NVM_OFFSET_ASSOCIATION_STATE);

                    /* Reset the supported model groups and save it to NVM */
                    /* Switch model */
                    MemSet(switch_model_groups, 0x0000,
                                            sizeof(switch_model_groups));
                    Nvm_Write((uint16 *)switch_model_groups, 
                                            sizeof(switch_model_groups),
                                            NVM_OFFSET_SWITCH_MODEL_GROUPS);

                    /* Attention model */
                    MemSet(attention_model_groups, 0x0000,
                                            sizeof(attention_model_groups));
                    Nvm_Write((uint16 *)attention_model_groups, 
                                            sizeof(attention_model_groups),
                                            NVM_OFFSET_ATTN_MODEL_GROUPS);
#ifdef ENABLE_DATA_MODEL
                    /* Data model */
                    MemSet(data_model_groups, 0x0000,
                                            sizeof(data_model_groups));
                    Nvm_Write((uint16 *)data_model_groups,
                                            sizeof(data_model_groups),
                                            NVM_OFFSET_DATA_MODEL_GROUPS);
#endif /* ENABLE_DATA_MODEL */

#ifdef ENABLE_WATCHDOG_MODEL
                    
                    /* Data model */
                    MemSet(wdog_model_groups, 0x0000,
                                            sizeof(wdog_model_groups));
                    Nvm_Write((uint16 *)wdog_model_groups,
                                            sizeof(wdog_model_groups),
                                            NVM_OFFSET_WDOG_MODEL_GROUPS);

                    AppWatchdogResetState();
#endif /* ENABLE_WATCHDOG_MODEL */

                    /* Association is removed from configuring device
                        initiate association once again */
                    InitiateAssociation();
                }
                break;
                case CSR_MESH_BEARER_STATE_EVENT:
                {
                    CSR_MESH_TRANSMIT_STATE_T bearer_tx_state;
                    CSR_MESH_BEARER_STATE_DATA_T bearer_state;
                    bearer_state = *((CSR_MESH_BEARER_STATE_DATA_T *)
                                        (eventDataCallback.appCallbackDataPtr));

                    g_switchapp_data.bearer_tx_state.bearerRelayActive 
                                                = bearer_state.bearerRelayActive;
                    g_switchapp_data.bearer_tx_state.bearerEnabled
                                                = bearer_state.bearerEnabled;
                    g_switchapp_data.bearer_tx_state.bearerPromiscuous
                                                = bearer_state.bearerPromiscuous;

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_switchapp_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    bearer_tx_state = g_switchapp_data.bearer_tx_state;
                    if(g_switchapp_data.gatt_data.st_ucid != GATT_INVALID_UCID)
                    {
                        /* Device is connected, so keep the promiscuous enabled. 
                         * Bearer State will be restored after disconnection.
                         */
                        bearer_tx_state.bearerPromiscuous = 
                                LE_BEARER_ACTIVE | GATT_SERVER_BEARER_ACTIVE;
                    }
                    else
                    {
                        /* Trigger connectable adverts. If the GATT bearer is
                         * enabled it will start advertising
                         */
                        GattTriggerConnectableAdverts();
                    }
                    AppUpdateBearerState(&bearer_tx_state);
                }
                break;
                case CSR_MESH_CONFIG_SET_PARAMS_EVENT:
                {

                }
                break;
                case CSR_MESH_GROUP_SET_MODEL_GROUPID_EVENT:
                {
                    HandleGroupSetMsg((*((CSR_MESH_GROUP_ID_RELATED_DATA_T *)
                        (eventDataCallback.appCallbackDataPtr))));
                }
                break;
                case CSR_MESH_ASSOCIATION_ATTENTION_EVENT:
                {
                    CSR_MESH_ASSOCIATION_ATTENTION_DATA_T *attn_data;

                    attn_data = (CSR_MESH_ASSOCIATION_ATTENTION_DATA_T *)
                                                 (eventDataCallback.appCallbackDataPtr);

                    /* Delete attention timer if it exists */
                    if (TIMER_INVALID != attn_tid)
                    {
                        TimerDelete(attn_tid);
                        attn_tid = TIMER_INVALID;
                    }
                    /* If attention Enabled */
                    if (attn_data->attract_attention)
                    {
                        /* Create attention duration timer if required */
                        if(attn_data->duration != 0xFFFF)
                        {
                            attn_tid = TimerCreate(attn_data->duration * MILLISECOND, 
                                                                TRUE, attnTimerHandler);
                        }
                        /* Enable Green light blinking to attract attention */
                        IOTLightControlDeviceBlink(0, 127, 0, 16, 16);
                    }
                    else
                    {
                        if(g_switchapp_data.assoc_state == app_state_not_associated)
                        {
                            /* Blink blue to indicate not associated status */
                            IOTLightControlDeviceBlink(0, 0, 127, 32, 32);
                        }
                        else
                        {
                            /* Restore the light Power State */
                            IOTLightControlDevicePower(FALSE);
                        }
                    }
                }
                break;
                case CSR_MESH_INVALID_EVENT:
                break;
                default:
                break;
            }
        }
        break;
        case CSR_MESH_OPERATION_STACK_NOT_INITIALIZED:
        break;
        case CSR_MESH_OPERATION_NOT_PERMITTED:
        break;
        case CSR_MESH_OPERATION_MEMORY_FULL:
        break;
        case CSR_MESH_OPERATION_GENERIC_FAIL:
        {
            switch(operation_event)
            {
                case CSR_MESH_ASSOC_COMPLETE_EVENT:
                case CSR_MESH_SEND_ASSOC_COMPLETE_EVENT:
                {
                    DEBUG_STR("\r\n Association is failed !!! \r\n");

                    /* Initiate association once again */
                    g_switchapp_data.assoc_state = app_state_not_associated;
                    InitiateAssociation();
                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_switchapp_data.assoc_state,
                             sizeof(g_switchapp_data.assoc_state),
                             NVM_OFFSET_ASSOCIATION_STATE);
                }
                break;
                default:
                break;
            }
        }
        break;
        case CSR_MESH_OPERATION_REQUEST_FOR_INFO:
        {
            switch(operation_event)
            {
                case CSR_MESH_GET_VID_PID_VERSTION_EVENT:
                {
                    DEBUG_STR("CSR_MESH_GET_VID_PID_VERSTION_EVENT\r\n");
                    MemCopy(((CSR_MESH_VID_PID_VERSION_T *)
                        (eventDataCallback.appCallbackDataPtr)),
                        &vid_pid_info,
                        sizeof(vid_pid_info));
                }
                break;
                case CSR_MESH_GET_DEVICE_APPEARANCE_EVENT:
                {
                    DEBUG_STR("CSR_MESH_GET_DEVICE_APPEARANCE_EVENT\r\n");
                    MemCopy(((CSR_MESH_DEVICE_APPEARANCE_T *)
                        (eventDataCallback.appCallbackDataPtr)),
                        &appearance,
                        sizeof(appearance));
                }
                break;
                case CSR_MESH_CONFIG_GET_PARAMS_EVENT:
                {
                }
                break;
                default:
                DEBUG_STR("APP:CSRMeshApplicationC : Unrecognised event<<\r\n");
                break;
            }
        }
        break;
        default:
            DEBUG_STR("APP:CSRMeshApplicationCb : Unrecognised optn <<\r\n");

    }

    //DEBUG_STR("\r\n Ref APP : CSRMeshApplicationCb <<\r\n");
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppLightEventHandler
 *
 *  DESCRIPTION
 *      Application function to handle CSRmesh Light Model messages
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
CSRmeshResult AppLightClientHandler(CSRMESH_MODEL_EVENT_T event_code,
                                    CSRMESH_EVENT_DATA_T* data,
                                    CsrUint16 length,
                                    void **state_data)
{
    switch(event_code)
    {
        case CSRMESH_LIGHT_STATE:
            DEBUG_STR("Received Light State\r\n");
        break;

        default:
        break;
    }
    return CSR_MESH_RESULT_SUCCESS;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppPowerEventHandler
 *
 *  DESCRIPTION
 *      Application function to handle CSRmesh Power Model messages
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
CSRmeshResult AppPowerClientHandler(CSRMESH_MODEL_EVENT_T event_code,
                                    CSRMESH_EVENT_DATA_T* data,
                                    CsrUint16 length,
                                    void **state_data)
{
    switch(event_code)
    {
        case CSRMESH_POWER_STATE:
            DEBUG_STR("Received Power State\r\n");
        break;
        
        default:
        break;
    }
    return CSR_MESH_RESULT_SUCCESS;
}
/*----------------------------------------------------------------------------*
 *  NAME
 *      AppAttentionEventHandler
 *
 *  DESCRIPTION
 *      Application function to handle CSRmesh Attention Model messages
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
CSRmeshResult AppAttentionEventHandler(CSRMESH_MODEL_EVENT_T event_code,
                                       CSRMESH_EVENT_DATA_T* data,
                                       CsrUint16 length,
                                       void **state_data)
{
    switch(event_code)
    {
        case CSRMESH_ATTENTION_SET_STATE:
        {
            CSRMESH_ATTENTION_SET_STATE_T *p_event = 
                 (CSRMESH_ATTENTION_SET_STATE_T *)data->data;

            g_switchapp_data.attn_model.attractattention = 
                                                    p_event->attractattention;
            g_switchapp_data.attn_model.duration = p_event->duration;
            g_switchapp_data.attn_model.tid = p_event->tid;

            /* Delete attention timer if it exists */
            if (TIMER_INVALID != attn_tid)
            {
                TimerDelete(attn_tid);
                attn_tid = TIMER_INVALID;
            }

            /* If attention Enabled */
            if (p_event->attractattention)
            {
                /* Create attention duration timer if required */
                if (p_event->duration != 0xFFFF)
                {
                    attn_tid = 
                        TimerCreate((uint32)p_event->duration * MILLISECOND,
                                                        TRUE, attnTimerHandler);
                }

                /* Enable Red light blinking to attract attention */
                LightHardwareSetBlink(127, 0, 0, 32, 32);
            }
            else
            {
                /* Turn of the LEDs */
                LightHardwarePowerControl(FALSE);
            }

            /* Send response data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_switchapp_data.attn_model;
            }

        }
        break;
        
        default:
        break;
    }

    return CSR_MESH_RESULT_SUCCESS;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppBatteryEventHandler
 *
 *  DESCRIPTION
 *      Application function to handle CSRmesh Battery Model messages
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
CSRmeshResult AppBatteryEventHandler(CSRMESH_MODEL_EVENT_T event_code, 
                                     CSRMESH_EVENT_DATA_T* data,
                                     CsrUint16 length,
                                     void **state_data)
{

    switch(event_code)
    {
        case CSRMESH_BATTERY_GET_STATE:
        {
            CSRMESH_BATTERY_GET_STATE_T *p_get_state = 
                                      (CSRMESH_BATTERY_GET_STATE_T *)data->data;

            /* Read Battery Level */
            g_switchapp_data.battery_model.batterylevel = ReadBatteryLevel();
            g_switchapp_data.battery_model.batterystate = GetBatteryState();
            g_switchapp_data.battery_model.tid = p_get_state->tid;

            /* Pass Battery state data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_switchapp_data.battery_model;
            }
        }
        break;

        default:
        break;
    }
    
    return CSR_MESH_RESULT_SUCCESS;
}


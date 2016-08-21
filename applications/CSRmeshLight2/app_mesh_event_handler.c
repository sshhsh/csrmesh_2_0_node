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
#include "csr_mesh_light.h"
#include "csr_mesh_light_gatt.h"
#include "csr_mesh_light_util.h"
#include "csr_mesh_light_hw.h"
#include "app_mesh_event_handler.h"
#include "battery_hw.h"
#include "csr_ota.h"
#include "csr_ota_service.h"
#include "gatt_service.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/
#define NUM_TRANSITION_STEPS       (100)



/* supported transition states */
typedef enum
{
    state_idle,
    state_color_change_attacking,
    state_level_change_attacking,
    state_sustaining,
    state_temp_change_attacking,
    state_decaying
}transition_sd_state;

/* Transition data stored for transition across different states */
typedef struct
{
    CsrUint32                   delta_duration;
    CsrUint32                   delta_sustain_duration;
    CsrUint32                   delta_decay_duration;

    CsrInt16                    stored_level;
    CsrInt16                    stored_red;
    CsrInt16                    stored_green;
    CsrInt16                    stored_blue;
    CsrInt16                    stored_temp;

    CsrInt16                    delta_level;
    CsrInt16                    delta_red;
    CsrInt16                    delta_green;
    CsrInt16                    delta_blue;
    CsrInt32                    delta_temp;

    transition_sd_state         transition_state;
    CsrUint8                    transition_count;
    timer_id                    transition_tid;
    CsrUint16                   dest_id;
}TRANSITION_DATA_T;

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

/* Transition data */
static TRANSITION_DATA_T g_trans_data;

/* Attention timer id */
static timer_id attn_tid = TIMER_INVALID;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

/* Power model Set state message handler */
static void togglePowerState(void);
static void lightDataNVMWriteTimerHandler(timer_id tid);
static uint32 getDurationForState(void);

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

        if(g_lightapp_data.assoc_state == app_state_associated)
        {
            /* Restore Light State */
            LightHardwareSetColor(g_lightapp_data.light_model.red,
                                  g_lightapp_data.light_model.green,
                                  g_lightapp_data.light_model.blue);
            LightHardwarePowerControl(g_lightapp_data.power_model.state);
        }
        else if(g_lightapp_data.assoc_state == app_state_association_started)
        {
            /* Blink Light in Yellow to indicate association in progress */
            IOTLightControlDeviceBlink(127, 127, 0, 32, 32);
        }
        else
        {
            /* Restart Blue blink to indicate ready for association */
            LightHardwareSetBlink(0, 0, 127, 32, 32);
        }
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      lightDataNVMWriteTimerHandler
 *
 *  DESCRIPTION
 *      This function handles NVM Write Timer time-out.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void lightDataNVMWriteTimerHandler(timer_id tid)
{
    uint32 rd_data = 0;
    uint32 wr_data = 0;

    if (tid == g_lightapp_data.nvm_tid)
    {
        g_lightapp_data.nvm_tid = TIMER_INVALID;

        /* Read RGB and Power Data from NVM */
        Nvm_Read((uint16 *)&rd_data, sizeof(uint32),
                 NVM_RGB_DATA_OFFSET);

        /* Pack Data for writing to NVM */
        wr_data = ((uint32) g_lightapp_data.power_model.state << 24) |
                  ((uint32) g_lightapp_data.light_model.blue  << 16) |
                  ((uint32) g_lightapp_data.light_model.green <<  8) |
                  g_lightapp_data.light_model.red;

        /* If data on NVM is not equal to current state, write current state
         * to NVM.
         */
        if (rd_data != wr_data)
        {
            Nvm_Write((uint16 *)&wr_data, sizeof(uint32),NVM_RGB_DATA_OFFSET);
        }
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      initialiseTransitionData
 *
 *  DESCRIPTION
 *      This function initialises the transition data
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void initialiseTransitionData(void)
{
    g_trans_data.transition_state = state_idle;
    g_trans_data.transition_count = 0;

    if(g_trans_data.transition_tid != TIMER_INVALID)
    {
        TimerDelete(g_trans_data.transition_tid);
    }
    g_trans_data.transition_tid = TIMER_INVALID;
    g_trans_data.delta_red = 0;
    g_trans_data.delta_green = 0;
    g_trans_data.delta_blue = 0;
    g_trans_data.delta_level = 0;

    g_trans_data.stored_red = 0;
    g_trans_data.stored_green = 0;
    g_trans_data.stored_blue = 0;
    g_trans_data.stored_level = 0;

    g_trans_data.delta_duration = 0;
    g_trans_data.delta_sustain_duration = 0;
    g_trans_data.delta_decay_duration = 0;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      getDurationForState
 *
 *  DESCRIPTION
 *      This function returns the duration based on the transition state
 *
 *  RETURNS/MODIFIES
 *      The delta duration for the corresponding transition state
 *
 *----------------------------------------------------------------------------*/
static uint32 getDurationForState(void)
{
    uint32 duration = 0;

    switch(g_trans_data.transition_state)
    {
        case state_color_change_attacking:
        case state_level_change_attacking:
        case state_temp_change_attacking:
            duration = g_trans_data.delta_duration;
        break;

        case state_sustaining:
            duration = g_trans_data.delta_sustain_duration;
        break;

        case state_decaying:
            duration = g_trans_data.delta_decay_duration;
        break;

        case state_idle:
        default:
        break;
    }
    return duration;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      transitionTimerHandler
 *
 *  DESCRIPTION
 *      This function handles the timer expiry for transition.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void transitionTimerHandler(timer_id tid)
{
    if (tid == g_trans_data.transition_tid)
    {
        g_trans_data.transition_tid = TIMER_INVALID;
        g_trans_data.transition_count ++;
        bool start_nvm_timer = FALSE;

        /* If the count is less than max transition steps then modify and set 
         * the model values based on the appropriate transition state and 
         * restart the transition timer again.
         */
        if(g_trans_data.transition_count <= NUM_TRANSITION_STEPS)
        {
            switch(g_trans_data.transition_state)
            {
                case state_temp_change_attacking:
                {
                    g_lightapp_data.light_model.colortemperature = 
                        g_trans_data.stored_temp + (uint16)((g_trans_data.delta_temp * 
                         g_trans_data.transition_count) /NUM_TRANSITION_STEPS);

                    /*LightHardwareGetRGBFromColorTemp(
                                   g_lightapp_data.light_model.colortemperature,
                                   &g_lightapp_data.light_model.red,
                                   &g_lightapp_data.light_model.blue,
                                   &g_lightapp_data.light_model.green);*/
                    /* Set the light level in the updated RGB setting */
                    LightHardwareSetLevel(g_lightapp_data.light_model.red,
                                          g_lightapp_data.light_model.green,
                                          g_lightapp_data.light_model.blue,
                                          g_lightapp_data.light_model.level);
                }
                break;
                case state_color_change_attacking:
                {
                    g_lightapp_data.light_model.red = 
                        (CsrUint8)(g_trans_data.stored_red +
                        (CsrInt16)(g_trans_data.delta_red * 
                        g_trans_data.transition_count) /NUM_TRANSITION_STEPS);

                    g_lightapp_data.light_model.green = 
                        (CsrUint8)(g_trans_data.stored_green +
                        (CsrInt16)(g_trans_data.delta_green * 
                        g_trans_data.transition_count) /NUM_TRANSITION_STEPS);

                    g_lightapp_data.light_model.blue = 
                        (CsrUint8)(g_trans_data.stored_blue +
                        (CsrInt16)(g_trans_data.delta_blue * 
                        g_trans_data.transition_count) /NUM_TRANSITION_STEPS);
                }
                /* Fall-Through */
                case state_level_change_attacking:
                case state_decaying:
                {
                    g_lightapp_data.light_model.level = 
                        (CsrUint8)(g_trans_data.stored_level +
                        (CsrInt16)(g_trans_data.delta_level * 
                        g_trans_data.transition_count) /NUM_TRANSITION_STEPS);

                    /* Set the light level in the latest RGB setting */
                    LightHardwareSetLevel(g_lightapp_data.light_model.red,
                                          g_lightapp_data.light_model.green,
                                          g_lightapp_data.light_model.blue,
                                          g_lightapp_data.light_model.level);

                    start_nvm_timer = TRUE;
                }
                break;
                case state_sustaining:
                case state_idle:
                default:
                break;
            }

            /* Restart the timer based on the transition state */
            g_trans_data.transition_tid =
                                        TimerCreate(getDurationForState(),
                                                    TRUE,
                                                    transitionTimerHandler);
            if(start_nvm_timer == TRUE)
            {
                /* Delete existing timer */
                if (TIMER_INVALID != g_lightapp_data.nvm_tid)
                {
                    TimerDelete(g_lightapp_data.nvm_tid);
                }

                /* Restart the timer */
                g_lightapp_data.nvm_tid = 
                                TimerCreate(NVM_WRITE_DEFER_DURATION,
                                            TRUE,
                                            lightDataNVMWriteTimerHandler);
            }
        }
        /* If the transition count has reached the maximum steps then start a 
         * new transition based on the state or move the state to idle if all 
         * the transitions are complete.
         */
        else
        {
            /* Send a light state with no ack message on a transition complete
             * when there is a change of model attributes and reset the 
             * transition count.
             */
            g_trans_data.transition_count = 0;
            if(g_trans_data.transition_state != state_sustaining)
            {
                LightState(DEFAULT_NW_ID,
                           g_trans_data.dest_id,
                           &g_lightapp_data.light_model, FALSE);
            }
            /* If the Level change transtition is complete then move to either
             * sustaining state or decaying state based on the delta duration
             * values.
             */
            if(g_trans_data.transition_state == state_level_change_attacking)
            {
                if(g_trans_data.delta_sustain_duration != 0)
                {
                    g_trans_data.transition_state = state_sustaining;
                }
                else if(g_trans_data.delta_decay_duration != 0)
                {
                    g_trans_data.transition_state = state_decaying;

                    g_trans_data.delta_level = 
                                (CsrInt16)(-g_lightapp_data.light_model.level);
                    g_trans_data.stored_level = 
                                            g_lightapp_data.light_model.level;
                }
                else
                {
                    g_trans_data.transition_state = state_idle;
                }
            }
            /* If the sustaining transtition is complete then move to decaying
             * state if the delta decay duration is not zero otherwise move to
             * idle state.
             */
            else if(g_trans_data.transition_state == state_sustaining)
            {
                if(g_trans_data.delta_decay_duration != 0)
                {
                    g_trans_data.transition_state = state_decaying;

                    g_trans_data.delta_level = 
                                (CsrInt16)(-g_lightapp_data.light_model.level);
                    g_trans_data.stored_level = 
                                            g_lightapp_data.light_model.level;
                }
                else
                {
                    g_trans_data.transition_state = state_idle;
                }
            }
            /* In all the other transition cases move back to idle state */
            else
            {
                g_trans_data.transition_state = state_idle;
            }

            /* start a new transition timer if the state is not assigned to 
             * idle for the next transition.
             */
            if(g_trans_data.transition_state != state_idle)
            {
                /* Restart the timer */
                g_trans_data.transition_tid =
                                        TimerCreate(getDurationForState(),
                                                    TRUE,
                                                    transitionTimerHandler);
            }
        }
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      togglePowerState
 *
 *  DESCRIPTION
 *      This function toggles the power state.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void togglePowerState(void)
{
    csr_mesh_power_state_t curr_state = g_lightapp_data.power_model.state;

    switch (curr_state)
    {
        case csr_mesh_power_state_on:
            g_lightapp_data.power_model.state = csr_mesh_power_state_off;
        break;

        case csr_mesh_power_state_off:
            g_lightapp_data.power_model.state = csr_mesh_power_state_on;
        break;

        case csr_mesh_power_state_onfromstandby:
            g_lightapp_data.power_model.state = csr_mesh_power_state_standby;
        break;

        case csr_mesh_power_state_standby:
            g_lightapp_data.power_model.state 
                                        = csr_mesh_power_state_onfromstandby;
        break;

        default:
        break;
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      CSRmeshAppProcessMeshEvent
 *
 *  DESCRIPTION
 *   The CSRmeshstack calls this call-back to notify asynchronous 
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
                    g_lightapp_data.assoc_state = app_state_association_started;

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
                    uint32 rd_data = 0;
                    uint32 wr_data = 0;

                    DEBUG_STR("\r\n Ref APP: CSR_MESH ASSOC_COMPLETE \r\n");
                    g_lightapp_data.assoc_state = app_state_associated;

                    /* Restore default light state */
                    LightHardwareSetColor(
                                    g_lightapp_data.light_model.red,
                                    g_lightapp_data.light_model.green,
                                    g_lightapp_data.light_model.blue);

                    LightHardwarePowerControl(
                                    g_lightapp_data.power_model.state);

                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_lightapp_data.assoc_state,
                             sizeof(g_lightapp_data.assoc_state),
                             NVM_OFFSET_ASSOCIATION_STATE);

                    /* Read RGB and Power Data from NVM */
                    Nvm_Read((uint16 *)&rd_data, sizeof(uint32),
                             NVM_RGB_DATA_OFFSET);

                    /* Disable promiscuous mode */
                    g_lightapp_data.bearer_tx_state.bearerPromiscuous = 0;

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_lightapp_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    /* If the device is connected as a bridge, the stored 
                     * promiscuous settings would be assigned at the time of 
                     * disconnection.
                     */
                    if(g_lightapp_data.gatt_data.st_ucid == GATT_INVALID_UCID)
                    {
                        AppUpdateBearerState(&g_lightapp_data.bearer_tx_state);
                    }

                    /* Pack Data for writing to NVM */
                    wr_data = 
                        ((uint32) g_lightapp_data.power_model.state << 24) |
                        ((uint32) g_lightapp_data.light_model.blue  << 16) |
                        ((uint32) g_lightapp_data.light_model.green <<  8) |
                        g_lightapp_data.light_model.red;

                    /* If data on NVM is not equal to current state, write 
                     * current state to NVM.
                     */
                    if (rd_data != wr_data)
                    {
                        Nvm_Write((uint16 *)&wr_data, 
                                            sizeof(uint32),NVM_RGB_DATA_OFFSET);
                    }
                }
                break;
                case CSR_MESH_CONFIG_RESET_DEVICE_EVENT:
                {
                    RemoveAssociation();
                }
                break;
                case CSR_MESH_BEARER_STATE_EVENT:
                {
                    CSR_MESH_TRANSMIT_STATE_T bearer_tx_state;
                    CSR_MESH_BEARER_STATE_DATA_T bearer_state;
                    bearer_state = *((CSR_MESH_BEARER_STATE_DATA_T *)
                                        (eventDataCallback.appCallbackDataPtr));

                    g_lightapp_data.bearer_tx_state.bearerRelayActive 
                                               = bearer_state.bearerRelayActive;
                    g_lightapp_data.bearer_tx_state.bearerEnabled
                                                = bearer_state.bearerEnabled;
                    g_lightapp_data.bearer_tx_state.bearerPromiscuous
                                               = bearer_state.bearerPromiscuous;

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_lightapp_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    bearer_tx_state = g_lightapp_data.bearer_tx_state;
                    if(g_lightapp_data.gatt_data.st_ucid != GATT_INVALID_UCID)
                    {
                        /* Device is connected, so keep the promiscuous enabled. 
                         * Bearer State will be restored after disconnection.
                         */
                        bearer_tx_state.bearerPromiscuous = 
                                     LE_BEARER_ACTIVE|GATT_SERVER_BEARER_ACTIVE;
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
                    HandleGroupSetMsg(( *((CSR_MESH_GROUP_ID_RELATED_DATA_T *)
                        (eventDataCallback.appCallbackDataPtr))));
                }
                break;
                case CSR_MESH_INVALID_EVENT:
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
                            attn_tid = TimerCreate(
                                            attn_data->duration * MILLISECOND, 
                                            TRUE, attnTimerHandler);
                        }
                        /* Enable Green light blinking to attract attention */
                        LightHardwareSetBlink(0, 127, 0, 16, 16);
                    }
                    else
                    {
                        if(g_lightapp_data.assoc_state == app_state_not_associated)
                        {
                            /* Blink blue to indicate not associated status */
                            LightHardwareSetBlink(0, 0, 127, 32, 32);
                        }
                        else
                        {
                            /* Restore Light State */
                            LightHardwareSetColor(
                                            g_lightapp_data.light_model.red,
                                            g_lightapp_data.light_model.green,
                                            g_lightapp_data.light_model.blue);
        
                            /* Restore the light Power State */
                            LightHardwarePowerControl(
                                            g_lightapp_data.power_model.state);
                        }
                    }
                }
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
                    g_lightapp_data.assoc_state = app_state_not_associated;
                    InitiateAssociation();
                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_lightapp_data.assoc_state,
                             sizeof(g_lightapp_data.assoc_state),
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
                            &vid_pid_info, sizeof(vid_pid_info));
                }
                break;
                case CSR_MESH_GET_DEVICE_APPEARANCE_EVENT:
                {
                    DEBUG_STR("CSR_MESH_GET_DEVICE_APPEARANCE_EVENT\r\n");
                    MemCopy(((CSR_MESH_DEVICE_APPEARANCE_T *)
                                    (eventDataCallback.appCallbackDataPtr)), 
                        &appearance, sizeof(appearance));
                }
                break;
                case CSR_MESH_CONFIG_GET_PARAMS_EVENT:
                {
                }
                break;
                default:
                DEBUG_STR("APP:CSRMeshApplicationCb :Unrecognised event<<\r\n");
                break;
            }
        }
        break;
		
        default:
            DEBUG_STR("APP:CSRMeshApplicationCb:Unrecognised opn status<<\r\n");

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
CSRmeshResult AppLightEventHandler(CSRMESH_MODEL_EVENT_T event_code,
                                   CSRMESH_EVENT_DATA_T* data,
                                   CsrUint16 length,
                                   void **state_data)
{
    bool start_nvm_timer = FALSE;

    switch(event_code)
    {
        /* handling of Light Model Event */
        case CSRMESH_LIGHT_SET_LEVEL_NO_ACK:
        case CSRMESH_LIGHT_SET_LEVEL:
        {
            CSRMESH_LIGHT_SET_LEVEL_T *p_data = 
            (CSRMESH_LIGHT_SET_LEVEL_T *)(((CSRMESH_EVENT_DATA_T *)data)->data);

            /* Initialise the transition data and delete the timer */
            initialiseTransitionData();

            g_lightapp_data.light_model.level = p_data->level;
            g_lightapp_data.light_model.power = csr_mesh_power_state_on;
            g_lightapp_data.power_model.state = csr_mesh_power_state_on;
            start_nvm_timer = TRUE;
            
            /* Set the light level */
            LightHardwareSetLevel(g_lightapp_data.light_model.red,
                                  g_lightapp_data.light_model.green,
                                  g_lightapp_data.light_model.blue,
                                  p_data->level);

            /* Send Light State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.light_model;
            }
        }
        break;


        case CSRMESH_LIGHT_SET_POWER_LEVEL_NO_ACK:
        case CSRMESH_LIGHT_SET_POWER_LEVEL:
        {
            CSRMESH_LIGHT_SET_POWER_LEVEL_T *p_data = 
                                (CSRMESH_LIGHT_SET_POWER_LEVEL_T *)
                                    (((CSRMESH_EVENT_DATA_T *)data)->data);
            uint32 duration = 0;

            /* Initialise the transition data and delete the timer.*/
            initialiseTransitionData();
            g_trans_data.dest_id = data->src_id;

            g_lightapp_data.light_model.power = p_data->power;
            g_lightapp_data.power_model.state = p_data->power;

            if(p_data->power == csr_mesh_power_state_on ||
               p_data->power == csr_mesh_power_state_onfromstandby)
            {
                /* convert the duration received into delta duration based on 
                 * the number of transition steps.
                 */
                g_trans_data.delta_duration =
                  (uint32)(p_data->levelduration*(SECOND/NUM_TRANSITION_STEPS));

                g_trans_data.delta_sustain_duration = 
                   (uint32)(p_data->sustain * (SECOND / NUM_TRANSITION_STEPS));

                g_trans_data.delta_decay_duration = 
                   (uint32)(p_data->decay * (SECOND / NUM_TRANSITION_STEPS));

                /* If the level duration is zero then update the level and move
                 * to sustain or decay states based on the sustain or decay 
                 * duration received.
                 */
                if(p_data->levelduration == 0)
                {
                    g_lightapp_data.light_model.level = p_data->level;
                    start_nvm_timer = TRUE;

                    /* Set the light level */
                    LightHardwareSetLevel(g_lightapp_data.light_model.red,
                                          g_lightapp_data.light_model.green,
                                          g_lightapp_data.light_model.blue,
                                          p_data->level);

                    /* If the sustain duration is non-zero move onto sustain .
                     * state. Even though we do not explicitely do any operation
                     * in the sustain state we need to split the sustain 
                     * duration onto delta timers as max timer supported is 
                     * around 32 min and the sustain duration could be higher.
                     */
                    if(p_data->sustain != 0)
                    {
                        g_trans_data.transition_state = state_sustaining;
                        duration = g_trans_data.delta_sustain_duration;
                    }
                    /* If neither the level duration or the sustain duration is 
                     * defined and decay duration is assigned then move onto 
                     * decay state.
                     */
                    else if(p_data->decay != 0)
                    {
                        g_trans_data.transition_state = state_decaying;
                        duration = g_trans_data.delta_decay_duration;
                        g_trans_data.delta_level = 
                            (CsrInt16)(-g_lightapp_data.light_model.level);
                        g_trans_data.stored_level = 
                                          g_lightapp_data.light_model.level;
                    }
                }
                /* If the level duration is defined then move to level_change 
                 * state.
                 */
                else
                {
                    g_trans_data.transition_state=state_level_change_attacking;
                    duration = g_trans_data.delta_duration;

                    g_trans_data.delta_level = (CsrInt16)(p_data->level - 
                                            g_lightapp_data.light_model.level);

                    g_trans_data.stored_level=g_lightapp_data.light_model.level;
               }

                /* Start the transition timer if the state is assigned to 
                 * non-idle.
                 */
                if(g_trans_data.transition_state != state_idle)
                {
                    /* start a timer and then start a counter to change the 
                     * color in NUM_TRANSITION_STEPS steps.
                     */
                    g_trans_data.transition_tid = 
                            TimerCreate(duration, TRUE, transitionTimerHandler);
                }
				
            }
            else if(p_data->power == csr_mesh_power_state_off ||
                    p_data->power == csr_mesh_power_state_standby)
            {
                LightHardwarePowerControl(FALSE);
            }

            /* Send Light State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.light_model;
            }
        }
        break;


        case CSRMESH_LIGHT_SET_RGB_NO_ACK:
        case CSRMESH_LIGHT_SET_RGB:
        {
			{
		CSR_MESH_APP_EVENT_DATA_T mydata;
		uint16 id = 0x2333;
		mydata.appCallbackDataPtr = &id;

		
        CSRmeshGetDeviceID(g_lightapp_data.netId,&mydata);
        /*id = *((uint16*)mydata.appCallbackDataPtr);*/
		DEBUG_U16(id);
		DEBUG_STR("\nthis is id\n");
		/*CSR_MESH_OPERATION_STATUS_T mystatus = mydata.status;
		switch(mystatus)
			{
			case CSR_MESH_OPERATION_SUCCESS:
				DEBUG_STR("success\n");
				break;
			case CSR_MESH_OPERATION_STACK_NOT_INITIALIZED:
				DEBUG_STR("stack not initialized\n");
				break;
			case CSR_MESH_OPERATION_NOT_PERMITTED:
				DEBUG_STR("operation not permited\n");
				break;
			case CSR_MESH_OPERATION_MEMORY_FULL:
				DEBUG_STR("memory full\n");
				break;
			case CSR_MESH_OPERATION_REQUEST_FOR_INFO:
				DEBUG_STR("request for info to app\n");
				break;
			case CSR_MESH_OPERATION_GENERIC_FAIL:
				DEBUG_STR("generic fail\n");
				break;
			default:
				break;
			}*/
			
    }
			
            CSRMESH_LIGHT_SET_RGB_T *p_data = 
                (CSRMESH_LIGHT_SET_RGB_T *)(data->data);

            g_lightapp_data.light_model.power = csr_mesh_power_state_on;
            g_lightapp_data.power_model.state = csr_mesh_power_state_on;

            /* Initialise the transition data and delete the timer */
            initialiseTransitionData();
            g_trans_data.dest_id = data->src_id;

            /* If the color duration is zero then update the RGB values. If the
             * duration is non-zero then move to color_change state and start 
             * the RGB and level transition to the desired values.
             */
            if(p_data->colorduration == 0)
            {
                g_lightapp_data.light_model.level = p_data->level;
                /* Update State of RGB in application */
                g_lightapp_data.light_model.red   = p_data->red;
                g_lightapp_data.light_model.green = p_data->green;
                g_lightapp_data.light_model.blue  = p_data->blue;
                start_nvm_timer = TRUE;

                /* Set the light level in the latest RGB setting */
                LightHardwareSetLevel(g_lightapp_data.light_model.red, 
                                      g_lightapp_data.light_model.green,
                                      g_lightapp_data.light_model.blue,
                                      g_lightapp_data.light_model.level);
            }
            else
            {
                g_trans_data.transition_state = state_color_change_attacking;

                /* Initialize the delta values by subtracting the received RGB
                 * and the level values to the ones present currently.
                 */
                g_trans_data.delta_level = (CsrInt16)(p_data->level - 
                                            g_lightapp_data.light_model.level);
                g_trans_data.delta_red   = (CsrInt16)(p_data->red - 
                                            g_lightapp_data.light_model.red);
                g_trans_data.delta_green = (CsrInt16)(p_data->green - 
                                            g_lightapp_data.light_model.green);
                g_trans_data.delta_blue = (CsrInt16)(p_data->blue - 
                                            g_lightapp_data.light_model.blue);

                /* store the current values as they are required for calculating
                 * during the transition process.
                 */
                g_trans_data.stored_level = g_lightapp_data.light_model.level;
                g_trans_data.stored_red = g_lightapp_data.light_model.red;
                g_trans_data.stored_green = g_lightapp_data.light_model.green;
                g_trans_data.stored_blue = g_lightapp_data.light_model.blue;


                g_trans_data.delta_duration =
                   (p_data->colorduration * (SECOND / NUM_TRANSITION_STEPS));

                /* start a timer and then start a counter to change the color
                 * in NUM_TRANSITION_STEPS steps.
                 */
                g_trans_data.transition_tid = 
                                TimerCreate(g_trans_data.delta_duration,
                                            TRUE,
                                            transitionTimerHandler);
            }

            /* Send Light State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.light_model;
            }
        }
        break;

        case CSRMESH_LIGHT_SET_COLOR_TEMP:
        {
#ifdef COLOUR_TEMP_ENABLED
            CSRMESH_LIGHT_SET_COLOR_TEMP_T *p_data = 
                            (CSRMESH_LIGHT_SET_COLOR_TEMP_T *)(data->data);
            
            g_lightapp_data.power_model.state = csr_mesh_power_state_on;
            g_lightapp_data.light_model.power = csr_mesh_power_state_on;

            /* Initialise the transition data and delete the timer */
            initialiseTransitionData();
            g_trans_data.dest_id = data->src_id;

            if(p_data->tempduration == 0)
            {
                g_lightapp_data.light_model.colortemperature = 
                                                    p_data->colortemperature;
            /* Set Colour temperature of light */
                LightHardwareSetColorTemp(
                                g_lightapp_data.light_model.colortemperature);
                start_nvm_timer = TRUE;
            }
            else
            {
                g_trans_data.transition_state = state_temp_change_attacking;
                /* Store current color temperature */
                g_trans_data.stored_temp = 
                                   g_lightapp_data.light_model.colortemperature;
                g_trans_data.delta_duration = 
                       (p_data->tempduration * (SECOND / NUM_TRANSITION_STEPS));

                g_trans_data.delta_temp = (int32)p_data->colortemperature -
                            (int32)g_lightapp_data.light_model.colortemperature;

                /* start a timer and then start a counter to change the color
                 * in NUM_TRANSITION_STEPS steps.
                 */
                g_trans_data.transition_tid = 
                                    TimerCreate(g_trans_data.delta_duration,
                                                TRUE,
                                                transitionTimerHandler);
            }
            /* Send Light State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.light_model;
            }
            
#endif /* COLOUR_TEMP_ENABLED */

        }
        break;
        
        case CSRMESH_LIGHT_GET_STATE:
        {
            /* Send Light State Information to Model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.light_model;
            }
        }
        break;

        default:
        break;
    }

    /* Start NVM timer if required */
    if (TRUE == start_nvm_timer)
    {
        /* Delete existing timer */
        if (TIMER_INVALID != g_lightapp_data.nvm_tid)
        {
            TimerDelete(g_lightapp_data.nvm_tid);
        }

        /* Restart the timer */
        g_lightapp_data.nvm_tid = TimerCreate(NVM_WRITE_DEFER_DURATION,
                                              TRUE,
                                              lightDataNVMWriteTimerHandler);
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
CSRmeshResult AppPowerEventHandler(CSRMESH_MODEL_EVENT_T event_code, 
                                   CSRMESH_EVENT_DATA_T* data,
                                   CsrUint16 length,
                                   void **state_data)
{
    bool state_changed = FALSE;

    switch(event_code)
    {
        case CSRMESH_POWER_SET_STATE_NO_ACK:
        case CSRMESH_POWER_SET_STATE:
        {
            CSRMESH_POWER_SET_STATE_T *p_event = 
                 (CSRMESH_POWER_SET_STATE_T *)
                                    (((CSRMESH_EVENT_DATA_T *)data)->data);
            if(g_lightapp_data.power_model.state != p_event->state)
            {
                g_lightapp_data.power_model.state = p_event->state;
                state_changed = TRUE;
            }
        }
        break;

        case CSRMESH_POWER_TOGGLE_STATE_NO_ACK:
        case CSRMESH_POWER_TOGGLE_STATE:
        {
            togglePowerState();
            state_changed = TRUE;
        }
        break;

        default:
        break;
    }

    if (TRUE == state_changed)
    {
        /* Set the light state accordingly */
        if (g_lightapp_data.power_model.state == 
                                            csr_mesh_power_state_off ||
            g_lightapp_data.power_model.state == 
                                            csr_mesh_power_state_standby)
        {
            LightHardwarePowerControl(FALSE);
        }
        else if(g_lightapp_data.power_model.state == 
                                            csr_mesh_power_state_on||
                g_lightapp_data.power_model.state == 
                                        csr_mesh_power_state_onfromstandby)
        {
            /* Turn on with stored colour */
            LightHardwareSetColor(g_lightapp_data.light_model.red,
                                  g_lightapp_data.light_model.green,
                                  g_lightapp_data.light_model.blue);

            LightHardwarePowerControl(TRUE);
        }

        /* Update the power state in the light model as well */
        g_lightapp_data.light_model.power = g_lightapp_data.power_model.state;

        /* Restart the NVM defer timer to save updated values after a delay */
        TimerDelete(g_lightapp_data.nvm_tid);
        g_lightapp_data.nvm_tid = TimerCreate(NVM_WRITE_DEFER_DURATION,
                                              TRUE,
                                              lightDataNVMWriteTimerHandler);
    }

    /* Return updated power state to the model */
    if (state_data != NULL)
    {
        *state_data = (void *)&g_lightapp_data.power_model;
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

            g_lightapp_data.attn_model.attractattention =
                                                    p_event->attractattention;
            g_lightapp_data.attn_model.duration = p_event->duration;
            g_lightapp_data.attn_model.tid = p_event->tid;

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
                /* Restore Light State */
                LightHardwareSetColor(g_lightapp_data.light_model.red,
                                      g_lightapp_data.light_model.green,
                                      g_lightapp_data.light_model.blue);

                /* Restore the light Power State */
                LightHardwarePowerControl(g_lightapp_data.power_model.state);
            }

            /* Send response data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.attn_model;
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
            g_lightapp_data.battery_model.batterylevel = ReadBatteryLevel();
            g_lightapp_data.battery_model.batterystate = GetBatteryState();
            g_lightapp_data.battery_model.tid = p_get_state->tid;

            /* Pass Battery state data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_lightapp_data.battery_model;
            }
        }
        break;

        default:
        break;
    }
    
    return CSR_MESH_RESULT_SUCCESS;
}


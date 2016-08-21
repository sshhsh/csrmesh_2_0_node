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
#include <gatt_uuid.h>

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
#include "csr_mesh_heater.h"
#include "csr_mesh_heater_gatt.h"
#include "csr_mesh_heater_util.h"
#include "app_mesh_event_handler.h"
#include "battery_hw.h"
#include "sensor_client.h"
#include "csr_ota.h"
#include "csr_ota_service.h"
#include "gatt_service.h"
/*============================================================================*
 *  Private Definitions
 *============================================================================*/

#ifdef ENABLE_ACK_MODE
typedef struct 
{
    uint16 dev_id;
    uint16 tid;
}DEVICE_INFO_T;

/* Maximum devices for which ackowledgements could be sent */
#define MAX_ACK_DEVICES                     (5)

/* Maximum retransmit count */
#define MAX_RETRANSMIT_COUNT                (MAX_RETRANSMISSION_TIME / \
                                             RETRANSMIT_INTERVAL)
#endif /* ENABLE_ACK_MODE */

typedef struct
{
    sensor_type_t type;
    uint16        *value;
} SENSOR_DATA_T;

typedef struct
{
    CSRMESH_SENSOR_TYPES_T   sensor_types;
    CSRMESH_SENSOR_VALUE_T   sensor_value;
} MODEL_RSP_DATA_T;

/* Current Air temperature sensor Index */
#define CURRENT_AIR_TEMP_IDX             (0)

/* Desired Air temperature sensor Index */
#define DESIRED_AIR_TEMP_IDX             (1)

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

/* Sensor Model Data */
static SENSOR_DATA_T sensor_data[NUM_SENSORS_SUPPORTED];

/* Model Response Common Data */
static MODEL_RSP_DATA_T model_rsp_data;

/* Temperature Value in 1/32 kelvin units. */
static SENSOR_FORMAT_TEMPERATURE_T current_air_temp;

/* Temperature Controller's Current Desired Air Temperature. */
static SENSOR_FORMAT_TEMPERATURE_T current_desired_air_temp;

/* Read value transmit counter */
static uint16 read_value_transmit_count = 0;

/* Read Value Timer ID. */
static timer_id read_val_tid = TIMER_INVALID;

#ifdef ENABLE_ACK_MODE
/* Retransmit Timer ID. */
static timer_id retransmit_tid = TIMER_INVALID;

/* Write Value Msg Retransmit counter */
static uint16   ack_retransmit_count = 0;

/* Array of structure holding the device id and the transaction id to be sent*/
static DEVICE_INFO_T sensor_dev_ack[MAX_ACK_DEVICES];
#endif /* ENABLE_ACK_MODE */

/* Attention timer id */
static timer_id attn_tid = TIMER_INVALID;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
static void updateHeaterStatus(void);
static void startReadValueTimer(void);
static void readValTimerHandler(timer_id tid);
static void readCurrentTempFromGroup(void);

#ifdef DEBUG_ENABLE
/* Print a number in decimal. */
static void printInDecimal(uint32 val);
#else
#define printInDecimal(n)
#endif /* DEBUG_ENABLE */

#ifdef ENABLE_ACK_MODE
static void sendValueAck(void);
static void retransmitIntervalTimerHandler(timer_id tid);
static void startRetransmitTimer(void);
static void addDeviceToSensorList(uint16 dev_id, uint8 tid);
static void resetDeviceList(void);
#endif /* ENABLE_ACK_MODE */

/*============================================================================*
 *  Private Function Definitions
 *============================================================================*/

#ifdef DEBUG_ENABLE
/*----------------------------------------------------------------------------*
 *  NAME
 *      printInDecimal
 *
 *  DESCRIPTION
 *      This function prints an UNSIGNED integer in decimal.
 *
 *  RETURNS
 *      Nothing.
 *
 
*----------------------------------------------------------------------------*/
static void printInDecimal(uint32 val)
{
    if(val >= 10)
    {
        printInDecimal(val/10);
    }
    DebugWriteChar(('0' + (val%10)));
}
#endif /* DEBUG_ENABLE */

#ifdef ENABLE_ACK_MODE
/*----------------------------------------------------------------------------*
 *  NAME
 *      sendValueAck
 *
 *  DESCRIPTION
 *      This function sends the sensor value acknowledgement message back to the
 *      device.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void sendValueAck(void)
{
    uint16 index;

    for(index = 0; index < MAX_ACK_DEVICES; index++)
    {
        if(sensor_dev_ack[index].dev_id != MESH_BROADCAST_ID)
        {
            /* Retransmitting the same message on every transmission interval 
             * can be configured to increase the possibility of the msg to reach 
             * to the scanning devices with more robustness as they are scanning 
             * at low duty cycles.
             */
            CSRMESH_SENSOR_VALUE_T value;
            value.type = sensor_type_internal_air_temperature;
            value.value[0] = WORD_LSB(current_air_temp);
            value.value[1] = WORD_MSB(current_air_temp);
            value.value_len = 2;
            value.type2 = sensor_type_desired_air_temperature;
            value.value2[0] = WORD_LSB(current_desired_air_temp);
            value.value2[1] = WORD_MSB(current_desired_air_temp);
            value.value2_len = 2;
            value.tid = sensor_dev_ack[index].tid;
            SensorValue(0, sensor_dev_ack[index].dev_id, &value);
        }
    }
    DEBUG_STR(" Acknowledge DESIRED TEMP : ");
    printInDecimal(current_desired_air_temp/32);
    DEBUG_STR(" kelvin\r\n");

    DEBUG_STR(" Acknowledge AIR TEMP : ");
    printInDecimal(current_air_temp/32);
    DEBUG_STR(" kelvin\r\n");
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      retransmitIntervalTimerHandler
 *
 *  DESCRIPTION
 *      This function expires when the next message needs to be transmitted 
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void retransmitIntervalTimerHandler(timer_id tid)
{
    if (tid == retransmit_tid)
    {
        retransmit_tid = TIMER_INVALID;

        /* transmit the pending message to all the groups*/
        sendValueAck();

        ack_retransmit_count --;

        /* start a timer to send the broadcast ack data */
        startRetransmitTimer();
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      startRetransmitTimer
 *
 *  DESCRIPTION
 *      This function starts the broadcast timer for retransmission.
 *
 *  RETURNS
 *      None
 *
 *----------------------------------------------------------------------------*/
static void startRetransmitTimer(void)
{
    if(retransmit_tid == TIMER_INVALID && ack_retransmit_count > 0)
    {
        retransmit_tid = TimerCreate(RETRANSMIT_INTERVAL, TRUE,
                                     retransmitIntervalTimerHandler);
    }

    /* reset the device id and the tid in the device list as we have sent all
     * the ack to the devices
     */
    if(ack_retransmit_count == 0)
    {
        resetDeviceList();
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      addDeviceToSensorList
 *
 *  DESCRIPTION
 *      This function adds the device id and the transaction id onto the 
 *      list.
 *
 *  RETURNS
 *      None
 *
 *----------------------------------------------------------------------------*/
static void addDeviceToSensorList(uint16 dev_id, uint8 tid)
{
    uint8 index;
    bool dev_found = FALSE;

    /* Check whether the device is already present in the list as we are 
     * sending the acknowledgements in which case we can refresh the latest
     * tid for that device.
     */
    for(index = 0; index < MAX_ACK_DEVICES; index++)
    {
        if(sensor_dev_ack[index].dev_id == dev_id)
        {
            dev_found = TRUE;
            sensor_dev_ack[index].tid = tid;
            break;
        }
    }
    /* If the device is not found then add the device onto the device list db */
    if(!dev_found)
    {
        for(index = 0; index < MAX_ACK_DEVICES; index++)
        {
            if(sensor_dev_ack[index].dev_id == MESH_BROADCAST_ID)
            {
                sensor_dev_ack[index].dev_id = dev_id;
                sensor_dev_ack[index].tid = tid;
                break;
            }
        }
        /* If the device list is full then just replace the oldest device id as
         * we would have sent the maximum acknowledgements to the first dev
         * added in the list.
         */
        if(index == MAX_ACK_DEVICES - 1)
        {
            sensor_dev_ack[0].dev_id = dev_id;
            sensor_dev_ack[0].tid = tid;
        }
    }
    ack_retransmit_count = MAX_RETRANSMIT_COUNT;

    /* start a timer to send the broadcast ack data */
    TimerDelete(retransmit_tid);
    retransmit_tid = TIMER_INVALID;
    startRetransmitTimer();
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      resetDeviceList
 *
 *  DESCRIPTION
 *      The function resets the device id and the ack flag of complete db
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void resetDeviceList(void)
{
    uint16 idx;

    for(idx=0; idx < MAX_ACK_DEVICES; idx++)
    {
        sensor_dev_ack[idx].dev_id = MESH_BROADCAST_ID;
        sensor_dev_ack[idx].tid = 0;
    }
}
#endif /* ENABLE_ACK_MODE */

/*----------------------------------------------------------------------------*
 *  NAME
 *      updateHeaterStatus
 *
 *  DESCRIPTION
 *      This function updates the Heater Status from ON or OFF
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void updateHeaterStatus(void)
{
    if( current_desired_air_temp > current_air_temp )
    {
        if( g_heater_app_data.status == heater_off )
        {
            /* Print only if the status changed */
            DEBUG_STR("HEATER ON\r\n");
            g_heater_app_data.status = heater_on;
        }
        /* Indicate the Heater ON status by glowing red LED */
        IOTLightControlDeviceSetColor(255,0,0);
    }
    else
    {
        if( g_heater_app_data.status == heater_on )
        {
            /* Print if the status changed */
            DEBUG_STR("HEATER OFF\r\n");
            g_heater_app_data.status = heater_off;
        }
        /* Turn off the red LED to indicate Heating status */
        IOTLightControlDevicePower(FALSE);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      readCurrentTempFromGroup
 *
 *  DESCRIPTION
 *      This function reads the current temperature from the supported group
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void readCurrentTempFromGroup(void)
{
    uint16 index;
    CSRMESH_SENSOR_READ_VALUE_T sensor_read;

    for(index = 0; index < NUM_SENSOR_MODEL_GROUPS; index++)
    {
        if(sensor_model_groups[index] != 0)
        {
            sensor_read.type = sensor_type_internal_air_temperature;
            sensor_read.type2 = sensor_type_desired_air_temperature;
            sensor_read.tid = 0;
            SensorReadValue(0,
                            sensor_model_groups[index], 
                            &sensor_read);
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      startReadValueTimer
 *
 *  DESCRIPTION
 *      This function starts the broadcast timer for retransmission.
 *
 *  RETURNS
 *      None
 *
 *----------------------------------------------------------------------------*/
static void startReadValueTimer(void)
{
    if(read_val_tid == TIMER_INVALID && read_value_transmit_count > 0)
    {
        read_val_tid = TimerCreate(RETRANSMIT_INTERVAL, TRUE,
                                   readValTimerHandler);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      readValTimerHandler
 *
 *  DESCRIPTION
 *      This function expires when the next message needs to be transmitted 
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void readValTimerHandler(timer_id tid)
{
    if (tid == read_val_tid)
    {
        read_val_tid = TIMER_INVALID;

        /* Read the internal and desired temp from the group */
        readCurrentTempFromGroup();

        read_value_transmit_count --;

        /* start a timer to read the temp from the group */
        startReadValueTimer();
    }
}

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

        /* Set back the scan to low duty cycle only if the device has
         * already been grouped.
         */
        EnableHighDutyScanMode(FALSE);

        /* Restore Light State */
        if(g_heater_app_data.assoc_state == app_state_associated)
        {
            if( g_heater_app_data.status == heater_off)
            {
                /* Heating is OFF turn off red LED */
                IOTLightControlDeviceSetColor(0,0,0);

                IOTLightControlDevicePower(FALSE);
            }
            else
            {
                /* Heating is ON so turn on red LED */
                IOTLightControlDeviceSetColor(255,0,0);
            }
        }
        else if(g_heater_app_data.assoc_state == app_state_association_started)
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

/*-----------------------------------------------------------------------------*
 *  NAME
 *      InitialiseSensorData
 *
 *  DESCRIPTION
 *      This function initialises supported sensor data.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
extern void InitialiseSensorData(void)
{
    /* Initialise sensor data. */
    sensor_data[CURRENT_AIR_TEMP_IDX].type        = 
                                        sensor_type_internal_air_temperature;
    sensor_data[CURRENT_AIR_TEMP_IDX].value       = 
                                        (uint16 *)&current_air_temp;

    sensor_data[DESIRED_AIR_TEMP_IDX].type        = 
                                        sensor_type_desired_air_temperature;
    sensor_data[DESIRED_AIR_TEMP_IDX].value       = 
                                        (uint16 *)&current_desired_air_temp;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      ReadSensorDataFromNVM
 *
 *  DESCRIPTION
 *      This function reads sensor state data from NVM into state variable.
 *
 *  RETURNS
 *      Nothing.
 *
 
*----------------------------------------------------------------------------*/
extern void ReadSensorDataFromNVM(uint16 idx)
{
    Nvm_Read((uint16*)(sensor_data[idx].value), 
             sizeof(uint16),
             (GET_SENSOR_NVM_OFFSET(idx)));
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteSensorDataToNVM
 *
 *  DESCRIPTION
 *      This function writes sensor state data from state variable into NVM.
 *
 *  RETURNS
 *      Nothing.
 *
 
*----------------------------------------------------------------------------*/
extern void WriteSensorDataToNVM(uint16 idx)
{
    Nvm_Write((uint16*)(sensor_data[idx].value), 
              sizeof(uint16),
              (GET_SENSOR_NVM_OFFSET(idx)));
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      IsHeaterConfigured
 *
 *  DESCRIPTION
 *      This below function returns whether the heater is configured or not
 *
 *  RETURNS/MODIFIES
 *      TRUE if the heater has been grouped otherwise returns FALSE
 *
 *----------------------------------------------------------------------------*/
extern bool IsHeaterConfigured(void)
{
    uint16 index;

    for(index = 0; index < NUM_SENSOR_MODEL_GROUPS; index++)
    {
        if(sensor_model_groups[index] != 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      ConfigureHeater
 *
 *  DESCRIPTION
 *      The below function is called when the sensor group is modified.
 *
 *  RETURNS/MODIFIES
 *      None
 *
 *----------------------------------------------------------------------------*/
extern void ConfigureHeater(bool old_config)
{
    /* If sensor was previously not grouped and has been grouped now, then the
     * sensor should move into low duty cycle 
     */
    if(!old_config && IsHeaterConfigured())
    {
        DEBUG_STR("Moving to Low Power Sniff Mode \r\n\r\n");
        EnableHighDutyScanMode(FALSE);
    }
    else if(old_config && !IsHeaterConfigured())
    {
        DEBUG_STR("Sensor Moving to active scan Mode \r\n\r\n");
        EnableHighDutyScanMode(TRUE);

#ifdef ENABLE_ACK_MODE
        /* Delete the retransmit timer */
        TimerDelete(retransmit_tid);
        retransmit_tid = TIMER_INVALID;
#endif
        /* Stop the reading of the temp */
        TimerDelete(read_val_tid);
        read_val_tid = TIMER_INVALID;
    }

    /* Grouping has been modified but sensor is still configured. Hence 
     * start temp read and update 
     */
    if(IsHeaterConfigured())
    {
        /* read the current temperature of the group */
        read_value_transmit_count = 60;

        /* start a timer to read the temp from the group */
        TimerDelete(read_val_tid);
        read_val_tid = TIMER_INVALID;
        startReadValueTimer();
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
        if(IsHeaterConfigured())
        {
            /* Change the Rx scan duty cycle to default val on disabling 
             * attention 
             */
            scan_duty_cycle = DEFAULT_RX_DUTY_CYCLE;
        }
        else
        {
            /* Change the Rx scan duty cycle to active as the device 
             * is not grouped yet.
             */
            scan_duty_cycle = HIGH_RX_DUTY_CYCLE;
        }
    }
    SetScanDutyCycle(scan_duty_cycle);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      InitialiseHeater
 *
 *  DESCRIPTION
 *      This function is called to enable the reading of the temperature sensor.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void InitialiseHeater(void)
{
    if(IsHeaterConfigured())
    {
        EnableHighDutyScanMode(FALSE);
        DEBUG_STR("Heater Configured Moving to Low Power Mode \r\n\r\n");

        /* read the current temperature of the group */
        read_value_transmit_count = 60;

#ifdef ENABLE_ACK_MODE
        retransmit_tid = TIMER_INVALID;
#endif
        /* start a timer to read the temp from the group */
        read_val_tid = TIMER_INVALID;
        startReadValueTimer();
    }
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
                    DEBUG_STR("\r\n APP: CSR_MESH_ASSOC_STARTED_EVENT \r\n");
                    g_heater_app_data.assoc_state =
                                                app_state_association_started;

                    /* Delete the attention timeout timer if enabled */
                    if( attn_tid != TIMER_INVALID)
                    {
                        TimerDelete(attn_tid);
                        attn_tid = TIMER_INVALID;
                    }

                    /* Blink Light in Yellow to indicate association started */
                    IOTLightControlDeviceBlink(127, 127, 0, 32, 32);

                }
                break;
                case CSR_MESH_ASSOC_COMPLETE_EVENT:
                case CSR_MESH_SEND_ASSOC_COMPLETE_EVENT:
                {
                    DEBUG_STR("\r\n APP: CSR_MESH ASSOC_COMPLETE \r\n");

                    g_heater_app_data.assoc_state = app_state_associated;

                    /* Stop LED blinking */
                    IOTLightControlDevicePower(FALSE);

                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_heater_app_data.assoc_state,
                             sizeof(g_heater_app_data.assoc_state),
                             NVM_OFFSET_ASSOCIATION_STATE);

                    /* Disable promiscuous mode */
                    g_heater_app_data.bearer_tx_state.bearerPromiscuous = 0;

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_heater_app_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    /* If the device is connected as a bridge, the stored 
                     * promiscuous settings would be assigned at the time of 
                     * disconnection.
                     */
                    if(g_heater_app_data.gatt_data.st_ucid == GATT_INVALID_UCID)
                    {
                        AppUpdateBearerState(&g_heater_app_data.bearer_tx_state);
                    }
                }
                break;
                case CSR_MESH_CONFIG_RESET_DEVICE_EVENT:
                {
                    uint16 index;

                    /* Enable promiscuous mode */
                    g_heater_app_data.bearer_tx_state.bearerPromiscuous = 
                                LE_BEARER_ACTIVE | GATT_SERVER_BEARER_ACTIVE;
                    AppUpdateBearerState(&g_heater_app_data.bearer_tx_state);

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_heater_app_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    /* Move device to dissociated state */
                    g_heater_app_data.assoc_state = app_state_not_associated;

                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_heater_app_data.assoc_state,
                             sizeof(g_heater_app_data.assoc_state),
                             NVM_OFFSET_ASSOCIATION_STATE);
#ifdef ENABLE_ACK_MODE
                    /* Delete the retransmit timer */
                    TimerDelete(retransmit_tid);
                    retransmit_tid = TIMER_INVALID;
#endif
                    /* Stop the reading of the temp */
                    TimerDelete(read_val_tid);
                    read_val_tid = TIMER_INVALID;

                    /* Reset the supported model groups and save it to NVM */
                    /* Sensor model */
                    MemSet(sensor_model_groups, 0x0000,
                                                sizeof(sensor_model_groups));

                    Nvm_Write(sensor_model_groups, sizeof(sensor_model_groups),
                                NVM_OFFSET_SENSOR_MODEL_GROUPS);

                    /* Attention model */
                    MemSet(attention_model_groups, 0x0000, 
                                                sizeof(attention_model_groups));
                    Nvm_Write(attention_model_groups, 
                                                sizeof(attention_model_groups),
                                                NVM_OFFSET_ATT_MODEL_GROUPS);
#ifdef ENABLE_DATA_MODEL
                    /* Data stream model */
                    MemSet(data_model_groups, 0x0000, 
                                                sizeof(data_model_groups));
                    Nvm_Write((uint16 *)data_model_groups,
                                                sizeof(data_model_groups),
                                                NVM_OFFSET_DATA_MODEL_GROUPS);
#endif /* ENABLE_DATA_MODEL */

                    /* Reset Sensor State. */
                    for (index = 0; index < NUM_SENSORS_SUPPORTED; index++)
                    {
                        WriteSensorDataToNVM(index);
                    }

                    current_air_temp = 0;
                    current_desired_air_temp = 0;

                    /* Start Mesh association again */
                    InitiateAssociation();
                }
                break;
                case CSR_MESH_BEARER_STATE_EVENT:
                {
                    CSR_MESH_BEARER_STATE_DATA_T bearer_state;
                    CSR_MESH_TRANSMIT_STATE_T bearer_tx_state;
                    bearer_state = *((CSR_MESH_BEARER_STATE_DATA_T *)
                                        (eventDataCallback.appCallbackDataPtr));

                    g_heater_app_data.bearer_tx_state.bearerRelayActive 
                                               = bearer_state.bearerRelayActive;
                    g_heater_app_data.bearer_tx_state.bearerEnabled
                                               = bearer_state.bearerEnabled;
                    g_heater_app_data.bearer_tx_state.bearerPromiscuous
                                               = bearer_state.bearerPromiscuous;

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_heater_app_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    bearer_tx_state = g_heater_app_data.bearer_tx_state;
                    if(g_heater_app_data.gatt_data.st_ucid != GATT_INVALID_UCID)
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
                    HandleGroupSetMsg(
                        ( *((CSR_MESH_GROUP_ID_RELATED_DATA_T *)
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
                        if(g_heater_app_data.assoc_state == app_state_not_associated)
                        {
                            /* Blink blue to indicate not associated status */
                            IOTLightControlDeviceBlink(0, 0, 127, 32, 32);
                        }
                        else
                        {
                            /* Restore the heater status display */
                            updateHeaterStatus();
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
                    DEBUG_STR("\r\n Association failed !!! \r\n");

                    /* Initiate association once again */
                    g_heater_app_data.assoc_state = app_state_not_associated;
                    InitiateAssociation();
                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_heater_app_data.assoc_state,
                             sizeof(g_heater_app_data.assoc_state),
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
                default:
                DEBUG_STR("APP: CSRMeshApplicationCb Unrecognised event<<\r\n");
                break;
            }
        }
        break;
        default:
            DEBUG_STR("APP: CSRMeshApplicationCb : Unrecognised operation\r\n");

    }

    //DEBUG_STR("\r\n Ref APP : CSRMeshApplicationCb <<\r\n");
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppSensorEventHandler
 *
 *  DESCRIPTION
 *      Application function to handle CSRmesh Sensor Model messages
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
CSRmeshResult AppSensorEventHandler(CSRMESH_MODEL_EVENT_T event_code, 
                                    CSRMESH_EVENT_DATA_T* data,
                                    CsrUint16 length, 
                                    void **state_data)
{
    switch(event_code)
    {
        /* Sensor Messages handling */
        case CSRMESH_SENSOR_GET_TYPES:
        {
            CSRMESH_SENSOR_GET_TYPES_T *p_event = 
                                    (CSRMESH_SENSOR_GET_TYPES_T *)data->data;
            MemSet(&model_rsp_data.sensor_types,
                   0x0000,
                   sizeof(model_rsp_data.sensor_types));

            if(p_event->firsttype <= sensor_type_internal_air_temperature)
            {
                model_rsp_data.sensor_types.types[0] = 
                                        sensor_type_internal_air_temperature;
                model_rsp_data.sensor_types.types[1] = 
                                        sensor_type_desired_air_temperature;
                model_rsp_data.sensor_types.types_len = 2;
            }
            else if(p_event->firsttype <= sensor_type_desired_air_temperature)
            {
                model_rsp_data.sensor_types.types[0] = 
                                        sensor_type_desired_air_temperature;
                model_rsp_data.sensor_types.types_len = 1;
            }
            else
            {
                model_rsp_data.sensor_types.types[0] = 
                                        sensor_type_desired_air_temperature;
                model_rsp_data.sensor_types.types_len = 0;
            }

            model_rsp_data.sensor_types.tid = p_event->tid;
            /* Send response data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&model_rsp_data.sensor_types;
            }
        }
        break;

        case CSRMESH_SENSOR_VALUE:
        case CSRMESH_SENSOR_WRITE_VALUE:
        case CSRMESH_SENSOR_WRITE_VALUE_NO_ACK:
        {
            CSRMESH_SENSOR_WRITE_VALUE_T *p_event = 
                                    (CSRMESH_SENSOR_WRITE_VALUE_T *)data->data;

            SENSOR_FORMAT_TEMPERATURE_T recvd_desired_temp = 0;
            SENSOR_FORMAT_TEMPERATURE_T recvd_air_temp = 0;
            uint8 *value, *value2;
            bool send_ack = FALSE;
            MemSet(&model_rsp_data.sensor_value,
                   0x0000,
                   sizeof(model_rsp_data.sensor_value));

            value = p_event->value;
            if(p_event->type == sensor_type_internal_air_temperature)
            {
                recvd_air_temp = (uint16)BufReadUint16(&value);
                CsrUint8 *p_temp = model_rsp_data.sensor_value.value;
                BufWriteUint16(&p_temp, recvd_air_temp);
                model_rsp_data.sensor_value.value_len = 2;
                send_ack = TRUE;
            }
            else if(p_event->type == sensor_type_desired_air_temperature) 
            {
                recvd_desired_temp = (uint16)BufReadUint16(&value);
                CsrUint8 *p_temp = model_rsp_data.sensor_value.value;
                BufWriteUint16(&p_temp, recvd_desired_temp);
                model_rsp_data.sensor_value.value_len = 2;
                send_ack = TRUE;
            }

            value2 = p_event->value2;
            if(p_event->type2 == sensor_type_desired_air_temperature) 
            {
                recvd_desired_temp = (uint16)BufReadUint16(&value2);
                CsrUint8 *p_temp = model_rsp_data.sensor_value.value2;
                BufWriteUint16(&p_temp, recvd_desired_temp);
                model_rsp_data.sensor_value.value2_len = 2;
                send_ack = TRUE;
            }
            else if(p_event->type2 == sensor_type_internal_air_temperature)
            {
                recvd_air_temp = (uint16)BufReadUint16(&value2);
                CsrUint8 *p_temp = model_rsp_data.sensor_value.value2;
                BufWriteUint16(&p_temp, recvd_air_temp);
                model_rsp_data.sensor_value.value2_len = 2;
                send_ack = TRUE;
            }

            /* We have received a new value from the group. Hence stop the 
             * read being done for the current and the desired temp.
             */
            TimerDelete(read_val_tid);
            read_val_tid = TIMER_INVALID;
            read_value_transmit_count = 0;

            if((current_desired_air_temp != recvd_desired_temp ||
                current_air_temp != recvd_air_temp) &&
                (recvd_air_temp != 0 && recvd_desired_temp != 0))
            {
                DEBUG_STR(" RECEIVED CURRENT TEMP : ");
                printInDecimal(recvd_air_temp/32);
                DEBUG_STR(" kelvin\r\n");
                DEBUG_STR(" RECEIVED DESIRED TEMP : ");
                printInDecimal(recvd_desired_temp/32);
                DEBUG_STR(" kelvin\r\n");

                /* As these are new values Initialise them to the latest */
                current_desired_air_temp = recvd_desired_temp;
                current_air_temp = recvd_air_temp;

                /* Desired or current air temperature has changed hence udpate
                 * heating status 
                 */
                updateHeaterStatus();

                /* Desired temperature needs to be updated in the NVM */
                WriteSensorDataToNVM(DESIRED_AIR_TEMP_IDX);

                if(event_code == CSRMESH_SENSOR_WRITE_VALUE)
                {
#ifdef ENABLE_ACK_MODE
                    addDeviceToSensorList(data->src_id, p_event->tid);
#endif /* ENABLE_ACK_MODE */
                    if(send_ack == TRUE)
                    {
                         model_rsp_data.sensor_value.type = p_event->type;
                         model_rsp_data.sensor_value.type2 = p_event->type2;
                         model_rsp_data.sensor_value.tid = p_event->tid;
                        /* Send response data to model */
                        if (state_data != NULL)
                        {
                            *state_data = (void *)&model_rsp_data.sensor_value;
                        }
                    }
                    else
                    {
                        *state_data = NULL;
                    }
                }
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

            g_heater_app_data.attn_model.attractattention = 
                                                    p_event->attractattention;
            g_heater_app_data.attn_model.duration = p_event->duration;
            g_heater_app_data.attn_model.tid = p_event->tid;

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
                    attn_tid = TimerCreate(
                                    (uint32)p_event->duration * MILLISECOND,
                                     TRUE,
                                     attnTimerHandler);
                }

                /* Enable Red light blinking to attract attention */
                IOTLightControlDeviceBlink(127, 0, 0, 32, 32);

                /* Change the Rx scan duty cycle on enabling attention */
                EnableHighDutyScanMode(TRUE);
            }
            else
            {
                /* Stop blink */
                IOTLightControlDevicePower(FALSE);

                /* Set back the scan to low duty cycle only if the device has
                 * already been grouped.
                 */
                EnableHighDutyScanMode(FALSE);
            }

            /* Send response data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_heater_app_data.attn_model;
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
            /* Batterymodel state data */
            CSRMESH_BATTERY_GET_STATE_T *p_get_state = 
                                      (CSRMESH_BATTERY_GET_STATE_T *)data->data;

            /* Read Battery Level */
            g_heater_app_data.battery_model.batterylevel = ReadBatteryLevel();
            g_heater_app_data.battery_model.batterystate = GetBatteryState();
            g_heater_app_data.battery_model.tid = p_get_state->tid;

            /* Pass Battery state data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_heater_app_data.battery_model;
            }
        }
        break;

        default:
        break;
    }
    
    return CSR_MESH_RESULT_SUCCESS;
}

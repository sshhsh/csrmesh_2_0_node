/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
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
#include <pio.h>
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
#include "csr_mesh_tempsensor.h"
#include "csr_mesh_tempsensor_gatt.h"
#include "csr_mesh_tempsensor_util.h"
#include "csr_mesh_tempsensor_hw.h"
#include "app_mesh_event_handler.h"
#include "battery_hw.h"
#include "sensor_client.h"
#include "csr_ota.h"
#include "csr_ota_service.h"
#include "gatt_service.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Absolute Difference of two numbers. */
#define ABS_DIFF(x,y) (((x) > (y))?((x) - (y)):((y) - (x)))

/* Desired Air temperature sensor Index */
#define DESIRED_AIR_TEMP_IDX                (1)

/* Current Air temperature sensor Index */
#define CURRENT_AIR_TEMP_IDX                (0)

/* Max sensor type supp in this app sensor_type_desired_air_temperature = 3*/
#define SENSOR_TYPE_SUPPORTED_MAX          (sensor_type_desired_air_temperature)

/* Max transmit msg density */
#define MAX_TRANSMIT_MSG_DENSITY            (6)

#ifdef ENABLE_ACK_MODE

/* The maximum number of heater devices that in grp */
#define MAX_HEATERS_IN_GROUP                (5)

/* The heater will be removed from the added list if it does not respond to 
 * maximim no response count times.
 */
#define MAX_NO_RESPONSE_COUNT               (5)
#endif /* ENABLE_ACK_MODE */

typedef struct
{
    sensor_type_t type;
    uint16        *value;
    uint8         repeat_interval;
} SENSOR_DATA_T;

#ifdef ENABLE_ACK_MODE
typedef struct 
{
    uint16 dev_id;
    bool ack_recvd;
    uint16 no_response_count;
}HEATER_INFO_T;
#endif /* ENABLE_ACK_MODE */

typedef struct
{
    CSRMESH_SENSOR_TYPES_T            sensor_types;
    CSRMESH_SENSOR_VALUE_T            sensor_value;
    CSRMESH_ACTUATOR_TYPES_T          actuator_types;
    CSRMESH_ACTUATOR_VALUE_ACK_T      actuator_value;
} MODEL_RSP_DATA_T;

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

/* Temperature Sensor Sample Timer ID. */
static timer_id tempsensor_sample_tid = TIMER_INVALID;

/* Retransmit Timer ID. */
static timer_id retransmit_tid = TIMER_INVALID;

/* Repeat Interval Timer ID. */
static timer_id repeat_interval_tid = TIMER_INVALID;

/* Write Value Msg Retransmit counter */
static uint16 write_val_retransmit_count = 0;

/* Temperature Value in 1/32 kelvin units. */
SENSOR_FORMAT_TEMPERATURE_T current_air_temp;

/* Last Broadcast temperature in 1/32 kelvin units. */
SENSOR_FORMAT_TEMPERATURE_T last_bcast_air_temp;

/* Temperature Controller's Current Desired Air Temperature. */
SENSOR_FORMAT_TEMPERATURE_T current_desired_air_temp;

/* Temperature Controller's Last Broadcast Desired Air Temperature. */
SENSOR_FORMAT_TEMPERATURE_T last_bcast_desired_air_temp;

/* Retransmit interval based on the msg transmit density 
 * These values are calculated based on the number of tx msgs added in queue.
 * TRANSMIT_MSG_DENSITY = 1 -> 90ms + (random 0-12.8ms) * 5 -> 500ms.(Approx)
 * TRANSMIT_MSG_DENSITY = 2 -> 45ms + (random 0-12.8ms) * 11 -> 700ms.(Approx)
 * TRANSMIT_MSG_DENSITY = 3 -> 30ms + (random 0-12.8ms) * 17 -> 800ms.(Approx)
 * TRANSMIT_MSG_DENSITY = 4 -> 22.5ms + (random 0-12.8ms) * 23 -> 900ms.(Approx)
 * TRANSMIT_MSG_DENSITY = 5 -> 20ms + (random 0-12.8ms) * 35 -> 1100ms.(Approx)
 * TRANSMIT_MSG_DENSITY = 6 -> 20ms + (random 0-12.8ms) * 41 -> 1400ms.(Approx)
 * tx msg density 6 used when transmit index is dynamically set when configured
 * to multiple groups.
 */
static uint32 retransmit_interval[MAX_TRANSMIT_MSG_DENSITY]={500 * MILLISECOND,
                                                             700 * MILLISECOND,
                                                             800 * MILLISECOND,
                                                             900 * MILLISECOND,
                                                            1100 * MILLISECOND,
                                                            1400 * MILLISECOND};

/* variable to store the msg transmit density */
static uint8 transmit_msg_density = TRANSMIT_MSG_DENSITY;

#ifdef ENABLE_ACK_MODE
/* Stores the device info of the heaters participating in the group.*/
static HEATER_INFO_T heater_list[MAX_HEATERS_IN_GROUP];
#endif /* ENABLE_ACK_MODE */

/* Attention timer id */
static timer_id attn_tid = TIMER_INVALID;

/* Model Response Common Data */
static MODEL_RSP_DATA_T model_rsp_data;

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/
static void writeTempValue(void);
static void startRetransmitTimer(void);
static void repeatIntervalTimerHandler(timer_id tid);

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
        if(g_tsapp_data.assoc_state == app_state_associated)
        {
            /* Stop blink */
            IOTLightControlDevicePower(FALSE);

            /* Set back the scan to low duty cycle only if the device has
             * already been grouped.
             */
            EnableHighDutyScanMode(FALSE);
        }
        else if(g_tsapp_data.assoc_state == app_state_association_started)
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

#ifdef ENABLE_ACK_MODE
/*----------------------------------------------------------------------------*
 *  NAME
 *      resetHeaterList
 *
 *  DESCRIPTION
 *      The function resets the device id and the ack flag of complete db
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void resetHeaterList(void)
{
    uint16 idx;

    for(idx=0; idx < MAX_HEATERS_IN_GROUP; idx++)
    {
        heater_list[idx].dev_id = MESH_BROADCAST_ID;
        heater_list[idx].ack_recvd = FALSE;
        heater_list[idx].no_response_count = 0;
    }
}
/*----------------------------------------------------------------------------*
 *  NAME
 *      resetAckInHeaterList
 *
 *  DESCRIPTION
 *      The function resets the ack flag of complete db with valid dev id
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void resetAckInHeaterList(void)
{
    uint16 idx;

    for(idx=0; idx < MAX_HEATERS_IN_GROUP; idx++)
    {
        if(heater_list[idx].dev_id != MESH_BROADCAST_ID)
        {
            heater_list[idx].ack_recvd = FALSE;
        }
    }
}
#endif /* ENABLE_ACK_MODE */

/*-----------------------------------------------------------------------------*
 *  NAME
 *      getSensorGroupCount
 *
 *  DESCRIPTION
 *      This below function returns the number of groups the sensor is 
 *      configured onto
 *
 *  RETURNS/MODIFIES
 *      The number of groups the sensor model is configured
 *
 *----------------------------------------------------------------------------*/
static uint8 getSensorGroupCount(void)
{
    uint8 index, num_groups=0;

    for(index = 0; index < NUM_SENSOR_MODEL_GROUPS; index++)
    {
        if(sensor_model_groups[index] != 0)
        {
            num_groups++;
        }
    }
    return num_groups;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      writeTempValue
 *
 *  DESCRIPTION
 *      This function writes the current and the desired temp values onto the 
 *      groups.
 *
 *  RETURNS
 *      Nothing.
 *
 
*----------------------------------------------------------------------------*/
static void writeTempValue(void)
{
    uint16 index, index1;
    bool ack_reqd = FALSE;
    CSRMESH_SENSOR_WRITE_VALUE_T sensor_values;

#ifdef ENABLE_ACK_MODE 
    ack_reqd = TRUE;
#endif /* ENABLE_ACK_MODE */

    for(index1 = 0; index1 < transmit_msg_density; index1 ++)
    {
        for(index = 0; index < NUM_SENSOR_MODEL_GROUPS; index++)
        {
            if(sensor_model_groups[index] != 0 && 
               current_air_temp != 0 &&
               current_desired_air_temp != 0)
            {
                /* Retransmitting same messages back to back multiple times 
                 * increases the probability of the message being received by 
                 * devices running on low duty cycle scan. 
                 * This can be tuned by setting the TRANSMIT_MESSAGE_DENSITY 
                 */
                /* transmit the pending message to all the groups */
                sensor_values.type = sensor_type_internal_air_temperature;
                sensor_values.value[0] = current_air_temp & 0xFF;
                sensor_values.value[1] = ((current_air_temp >> 8) & 0xFF);
                sensor_values.value_len = 2;
                sensor_values.type2 = sensor_type_desired_air_temperature;
                sensor_values.value2[0] = current_desired_air_temp & 0xFF;
                sensor_values.value2[1] = ((current_desired_air_temp >> 8) & 0xFF);
                sensor_values.value2_len= 2;
                sensor_values.tid = 0;

                SensorWriteValue(0,
                                 sensor_model_groups[index],
                                 &sensor_values,
                                 ack_reqd);
            }
        }
    }
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
        bool start_timer = TRUE;

        retransmit_tid = TIMER_INVALID;

        /* transmit the pending message to all the groups*/
        writeTempValue();

        write_val_retransmit_count --;

#ifdef ENABLE_ACK_MODE 
        /* After half of the max retransmissions are over then check whether
         * ack has been received from all the heaters stored and if so then
         * stop sending the packet as we have received acks for all heaters.
         */
        if(write_val_retransmit_count < (NUM_OF_RETRANSMISSIONS/2))
        {
            uint8 idx;
            for(idx=0; idx < MAX_HEATERS_IN_GROUP; idx++)
            {
                if(heater_list[idx].dev_id != MESH_BROADCAST_ID &&
                   heater_list[idx].ack_recvd == FALSE)
                {
                    break;
                }
                if(idx == (MAX_HEATERS_IN_GROUP-1))
                {
                    start_timer = FALSE;
                    DEBUG_STR(" RECVD ALL ACK'S STOP TIMER : ");
                }
            }
            /* One or more devices have not acked back increase the no response
             * count. If the no response count reaches the maximum, remove the
             * device from the heater list.
             */
            if(write_val_retransmit_count == 0)
            {
                for(idx=0; idx < MAX_HEATERS_IN_GROUP; idx++)
                {
                    if(heater_list[idx].dev_id != MESH_BROADCAST_ID &&
                       heater_list[idx].ack_recvd == FALSE)
                    {
                        heater_list[idx].no_response_count++;
                        if(heater_list[idx].no_response_count >= 
                                                        MAX_NO_RESPONSE_COUNT)
                        {
                            heater_list[idx].dev_id = MESH_BROADCAST_ID;
                            heater_list[idx].no_response_count = 0;
                        }
                    }
                }
            }
        }
#endif /* ENABLE_ACK_MODE */ 
 
        if(start_timer == TRUE)
        {
            /* start a timer to send the broadcast sensor data */
            startRetransmitTimer();
        }
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      startRetransmitTimer
 *
 *  DESCRIPTION
 *      This function starts the broadcast timer for the current tempertature.
 *
 *  RETURNS
 *      None
 *
 
*----------------------------------------------------------------------------*/
static void startRetransmitTimer(void)
{
    if(write_val_retransmit_count > 0 && getSensorGroupCount() > 0)
    {
        uint8 transmit_index = 
                    (transmit_msg_density * getSensorGroupCount()) -1;

        retransmit_tid=TimerCreate(
                     retransmit_interval[transmit_index],
                     TRUE,
                     retransmitIntervalTimerHandler);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      tempSensorSampleIntervalTimeoutHandler
 *
 *  DESCRIPTION
 *      This function handles the sensor sample interval time-out.
 *
 *  RETURNS
 *      Nothing.
 *
 
*----------------------------------------------------------------------------*/
static void tempSensorSampleIntervalTimeoutHandler(timer_id tid)
{
    if (tid == tempsensor_sample_tid)
    {
        tempsensor_sample_tid = TIMER_INVALID;

        /* Issue a Temperature Sensor Read. */
        TempSensorRead();

        /* Start the timer for next sample. */
        tempsensor_sample_tid = TimerCreate(
                                        (uint32)TEMPERATURE_SAMPLING_INTERVAL, 
                                        TRUE,
                                        tempSensorSampleIntervalTimeoutHandler);
    }
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      tempSensorEvent
 *
 *  DESCRIPTION
 *      This function Handles the sensor read complete event. Checks if the new 
 *      temperature is within tolerence from the last broadcast value, otherwise
 *      broadcasts new value.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
static void tempSensorEvent(int16 temp)
{
    /* Check if the new value is valid. */
    if (temp > 0)
    {
        uint16 abs_diff;
        SENSOR_FORMAT_TEMPERATURE_T cur_temp_returned = temp;

        /* If Desired air temperature is not Initialised, Initialise it based 
         * on current air temperature.
         */
        if (current_desired_air_temp == 0x0000)
        {
            current_desired_air_temp = cur_temp_returned;
        }

        DEBUG_STR(" CURR AIR TEMP read : ");
        PrintInDecimal(cur_temp_returned/32);
        DEBUG_STR(" kelvin\r\n");

        /* Find the absolute difference between current and last broadcast
         * temperatures.
         */
        abs_diff = ABS_DIFF((uint16)last_bcast_air_temp, cur_temp_returned);

        /* If it changed beyond the tolerance value, then write the current 
         * temp onto the group.
         */
        if (abs_diff >= TEMPERATURE_CHANGE_TOLERANCE)
        {
            /* Set the present temperature as it is more than the temperature
             * tolerance change. Write the new value to the group as well as
             * reset the retransmit count to max and start the retransmit timer
             * if its not started.
             */
            current_air_temp = cur_temp_returned;

            /* Set last Broadcast Temperature to current temperature. */
            last_bcast_air_temp = current_air_temp;

            StartTempTransmission();
        }
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      startRepeatIntervalTimer
 *
 *  DESCRIPTION
 *      Start the repeat interval timer  The function should be called only if
 *      the repeat interval on the desired or the current index is non zero.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void startRepeatIntervalTimer (void)
{
    /* The app takes the minimum value of the repeat interval from current and
     * desired repeat intervals.
     */
    uint8 interval = sensor_data[CURRENT_AIR_TEMP_IDX].repeat_interval;

    if(sensor_data[CURRENT_AIR_TEMP_IDX].repeat_interval == 0 || 
      (sensor_data[DESIRED_AIR_TEMP_IDX].repeat_interval !=0 && 
        sensor_data[DESIRED_AIR_TEMP_IDX].repeat_interval < 
        sensor_data[CURRENT_AIR_TEMP_IDX].repeat_interval))
    {
        interval = sensor_data[DESIRED_AIR_TEMP_IDX].repeat_interval;
    }

    /* As the application transmits the same message for a longer period of time
     * we would consider the repeat interval values below 30 seconds to be 
     * atleast 30 seconds.
     */
    if(interval < 30)
    {
        interval = 30;
    }

    /* Delete the repeat interval timer */
    TimerDelete(repeat_interval_tid);
    repeat_interval_tid = TIMER_INVALID;

    repeat_interval_tid = TimerCreate(((uint32)interval* SECOND),
            TRUE, repeatIntervalTimerHandler);

    /* Start the transmission once here */
    StartTempTransmission();
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      repeatIntervalTimerHandler
 *
 *  DESCRIPTION
 *      This function handles repeat interval time-out.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void repeatIntervalTimerHandler(timer_id tid)
{
    if (repeat_interval_tid == tid)
    {
        repeat_interval_tid = TIMER_INVALID;
        startRepeatIntervalTimer();
    }
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*-----------------------------------------------------------------------------*
 *  NAME
 *      InitiliseSensorData
 *
 *  DESCRIPTION
 *      This function initialises supported sensor data.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
extern void InitiliseSensorData(void)
{
    /* Initialise Temperature Sensor Hardware. */
    if (!TempSensorHardwareInit(tempSensorEvent))
    {
        DEBUG_STR("\r\nFailed to Initialise temperature sensor\r\n");
    }

    /* Initialise Application specific Sensor Model Data.
     * This needs to be done before readPersistentStore.
         */
    sensor_data[CURRENT_AIR_TEMP_IDX].repeat_interval = 
                                        DEFAULT_REPEAT_INTERVAL & 0xFF;
    sensor_data[CURRENT_AIR_TEMP_IDX].type        = 
                                        sensor_type_internal_air_temperature;
    sensor_data[CURRENT_AIR_TEMP_IDX].value       = 
                                        (uint16 *)&current_air_temp;

    sensor_data[DESIRED_AIR_TEMP_IDX].repeat_interval = 
                                                DEFAULT_REPEAT_INTERVAL & 0xFF;
    sensor_data[DESIRED_AIR_TEMP_IDX].type        = 
                                        sensor_type_desired_air_temperature;
    sensor_data[DESIRED_AIR_TEMP_IDX].value       = 
                                        (uint16 *)&current_desired_air_temp;

    tempsensor_sample_tid = TIMER_INVALID;
    retransmit_tid  = TIMER_INVALID;
    repeat_interval_tid = TIMER_INVALID;

#ifdef ENABLE_ACK_MODE 
    resetHeaterList();
#endif /* ENABLE_ACK_MODE */
}


/*----------------------------------------------------------------------------*
 * NAME
 *      StartTempTransmission
 *
 * DESCRIPTION
 *      This function starts the transmission of the temp sensor value.
 *
 * RETURNS
 *      Nothing.
 *
 
*----------------------------------------------------------------------------*/
extern void StartTempTransmission(void)
{
    transmit_msg_density = TRANSMIT_MSG_DENSITY;

    switch(getSensorGroupCount())
    {
        case 2:
            if(transmit_msg_density > 3)
                transmit_msg_density = 3;
            break;

        case 3:
            if(transmit_msg_density > 2)
                transmit_msg_density = 2;
        break;

        case 4:
            transmit_msg_density = 1;
        break;

        default:
        break;
    }

    write_val_retransmit_count = (NUM_OF_RETRANSMISSIONS/transmit_msg_density);

    DEBUG_STR(" RETRANSMIT_COUNT : ");
    PrintInDecimal(write_val_retransmit_count);
    DEBUG_STR("\r\n");

    DEBUG_STR(" TRANSMIT_MSG_DENSITY : ");
    PrintInDecimal(transmit_msg_density);
    DEBUG_STR("\r\n");

    DEBUG_STR(" WRITE DESIRED TEMP : ");
    PrintInDecimal(current_desired_air_temp/32);
    DEBUG_STR(" kelvin\r\n");

    DEBUG_STR(" WRITE AIR TEMP : ");
    PrintInDecimal(current_air_temp/32);
    DEBUG_STR(" kelvin\r\n");
    writeTempValue();
    write_val_retransmit_count --;

    TimerDelete(retransmit_tid);
    retransmit_tid  = TIMER_INVALID;
    startRetransmitTimer();

    WriteSensorDataToNVM(DESIRED_AIR_TEMP_IDX);
#ifdef ENABLE_ACK_MODE 
    resetAckInHeaterList();
#endif /* ENABLE_ACK_MODE */
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

    Nvm_Read((uint16*)&(sensor_data[idx].repeat_interval), 
             sizeof(uint8),
             (GET_SENSOR_NVM_OFFSET(idx) + sizeof(uint8)));

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

    Nvm_Write((uint16*)&(sensor_data[idx].repeat_interval), 
              sizeof(uint8),
              (GET_SENSOR_NVM_OFFSET(idx) + sizeof(uint8)));
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      IsSensorConfigured
 *
 *  DESCRIPTION
 *      This below function returns whether the sensor is configured or not
 *
 *  RETURNS/MODIFIES
 *      TRUE if the sensor has been grouped otherwise returns FALSE
 *
 *----------------------------------------------------------------------------*/
extern bool IsSensorConfigured(void)
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
 *      ConfigureSensor
 *
 *  DESCRIPTION
 *      The below function is called when the sensor group is modified.
 *
 *  RETURNS/MODIFIES
 *      None
 *
 *----------------------------------------------------------------------------*/
extern void ConfigureSensor(bool old_config)
{
    /* If sensor was previously not grouped and has been grouped now, then the
     * sensor should move into low duty cycle 
     */
    if(!old_config && IsSensorConfigured())
    {
#ifdef ENABLE_TEMPERATURE_CONTROLLER
        /* Print a message for temperature control. */
        DEBUG_STR("\r\nPress '+'/'-' Increase/Decrease Temp.\r\n");
#endif /* ENABLE_TEMPERATURE_CONTROLLER */

        EnableHighDutyScanMode(FALSE);
        DEBUG_STR("Moving to Low Power Sniff Mode \r\n\r\n");

        if(sensor_data[DESIRED_AIR_TEMP_IDX].repeat_interval !=0 ||
           sensor_data[CURRENT_AIR_TEMP_IDX].repeat_interval !=0)
        {
            startRepeatIntervalTimer();
        }
    }
    else if(old_config && !IsSensorConfigured())
    {
        DEBUG_STR("Sensor Moving to active scan Mode \r\n\r\n");
        EnableHighDutyScanMode(TRUE);

        /* Delete the repeat interval timer */
        TimerDelete(repeat_interval_tid);
        repeat_interval_tid = TIMER_INVALID;

        /* Stop the periodic reading of the temp */
        TimerDelete(tempsensor_sample_tid);
        tempsensor_sample_tid = TIMER_INVALID;

        /* Stop the retransmissions if already in progress */
        TimerDelete(retransmit_tid);
        retransmit_tid = TIMER_INVALID;
    }

    /* Grouping has been modified but sensor is still configured. Hence 
     * start temp read and update 
     */
    if(IsSensorConfigured())
    {
        /* Reset last bcast temp to trigger temp update on sensor read*/
        last_bcast_air_temp = 0;

        /* Issue a Read to start sampling timer. */
        TempSensorRead();

        /* Stop the retransmissions if already in progress */
        TimerDelete(retransmit_tid);
        retransmit_tid = TIMER_INVALID;

        /* Delete the temp sensor tid and create a new one */
        TimerDelete(tempsensor_sample_tid);
        tempsensor_sample_tid = TIMER_INVALID;

        /* Start the timer for next sample. */
        tempsensor_sample_tid = TimerCreate(
                                (uint32)TEMPERATURE_SAMPLING_INTERVAL, 
                                TRUE,
                                tempSensorSampleIntervalTimeoutHandler);
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
        if(IsSensorConfigured())
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
 *      EnableTempReading
 *
 *  DESCRIPTION
 *      This function is called to enable the reading of the temperature sensor.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void EnableTempReading(void)
{
   if(IsSensorConfigured())
   {
        DEBUG_STR("Temp Sensor Configured, Moving to Low Power Mode\r\n");

        EnableHighDutyScanMode(FALSE);

        /* Issue a Read to start sampling timer. */
        TempSensorRead();

        /* Start the timer for next sample. */
        tempsensor_sample_tid = TimerCreate(
                                    (uint32)TEMPERATURE_SAMPLING_INTERVAL, 
                                    TRUE,
                                    tempSensorSampleIntervalTimeoutHandler);

        if(sensor_data[DESIRED_AIR_TEMP_IDX].repeat_interval !=0 ||
           sensor_data[CURRENT_AIR_TEMP_IDX].repeat_interval !=0)
        {
            startRepeatIntervalTimer();
        }
    }
    else
    {
        DEBUG_STR("Temp Sensor not grouped\r\n");
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
                    g_tsapp_data.assoc_state = app_state_association_started;

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

                    g_tsapp_data.assoc_state = app_state_associated;

                    /* Stop LED blinking */
                    IOTLightControlDevicePower(FALSE);

                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_tsapp_data.assoc_state,
                             sizeof(g_tsapp_data.assoc_state),
                             NVM_OFFSET_ASSOCIATION_STATE);

                    /* Disable promiscuous mode */
                    g_tsapp_data.bearer_tx_state.bearerPromiscuous = 0;

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_tsapp_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    /* If the device is connected as a bridge, the stored 
                     * promiscuous settings would be assigned at the time of 
                     * disconnection.
                     */
                    if(g_tsapp_data.gatt_data.st_ucid == GATT_INVALID_UCID)
                    {
                        AppUpdateBearerState(&g_tsapp_data.bearer_tx_state);
                    }
                }
                break;
                case CSR_MESH_CONFIG_RESET_DEVICE_EVENT:
                {
                    uint16 index;

                    /* Enable promiscuous mode */
                    g_tsapp_data.bearer_tx_state.bearerPromiscuous = 
                                LE_BEARER_ACTIVE | GATT_SERVER_BEARER_ACTIVE;
                    AppUpdateBearerState(&g_tsapp_data.bearer_tx_state);

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_tsapp_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    /* Move device to dissociated state */
                    g_tsapp_data.assoc_state = app_state_not_associated;

                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_tsapp_data.assoc_state,
                             sizeof(g_tsapp_data.assoc_state),
                             NVM_OFFSET_ASSOCIATION_STATE);

                    /* Delete Temperature Sensor Timers. */
                    TimerDelete(retransmit_tid);
                    retransmit_tid = TIMER_INVALID;

                    TimerDelete(tempsensor_sample_tid);
                    tempsensor_sample_tid = TIMER_INVALID;

                    TimerDelete(repeat_interval_tid);
                    repeat_interval_tid = TIMER_INVALID;

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
                    last_bcast_air_temp  = 0;
                    current_desired_air_temp = 0;
                    last_bcast_desired_air_temp = 0;

                    /* Start Mesh association again */
                    InitiateAssociation();
                }
                break;
                case CSR_MESH_BEARER_STATE_EVENT:
                {
                    CSR_MESH_TRANSMIT_STATE_T bearer_tx_state;
                    CSR_MESH_BEARER_STATE_DATA_T bearer_state;
                    bearer_state = *((CSR_MESH_BEARER_STATE_DATA_T *)
                                        (eventDataCallback.appCallbackDataPtr));

                    g_tsapp_data.bearer_tx_state.bearerRelayActive 
                                                = bearer_state.bearerRelayActive;
                    g_tsapp_data.bearer_tx_state.bearerEnabled
                                                = bearer_state.bearerEnabled;
                    g_tsapp_data.bearer_tx_state.bearerPromiscuous
                                                = bearer_state.bearerPromiscuous;

                    /* Save the state to NVM */
                    Nvm_Write((uint16 *)&g_tsapp_data.bearer_tx_state, 
                              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                              NVM_OFFSET_BEARER_STATE);

                    bearer_tx_state = g_tsapp_data.bearer_tx_state;
                    if(g_tsapp_data.gatt_data.st_ucid != GATT_INVALID_UCID)
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
                            attn_tid = TimerCreate(attn_data->duration * MILLISECOND, 
                                                                TRUE, attnTimerHandler);
                        }
                        /* Enable Green light blinking to attract attention */
                        IOTLightControlDeviceBlink(0, 127, 0, 16, 16);
                    }
                    else
                    {
                        if(g_tsapp_data.assoc_state == app_state_not_associated)
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
                    g_tsapp_data.assoc_state = app_state_not_associated;
                    InitiateAssociation();
                    /* Write association state to NVM */
                    Nvm_Write((uint16 *)&g_tsapp_data.assoc_state,
                             sizeof(g_tsapp_data.assoc_state),
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
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      AppActuatorEventHandler
 *
 *  DESCRIPTION
 *      Application function to handle CSRmesh Actuator Model messages
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
CSRmeshResult AppActuatorEventHandler(CSRMESH_MODEL_EVENT_T event_code, 
                                    CSRMESH_EVENT_DATA_T* data,
                                    CsrUint16 length, 
                                    void **state_data)
{
    switch(event_code)
    {
        /* Actuator Messages handling */
        case CSRMESH_ACTUATOR_GET_TYPES:
        {
            CSRMESH_ACTUATOR_GET_TYPES_T *p_event = 
                                    (CSRMESH_ACTUATOR_GET_TYPES_T *)data->data;
            MemSet(&model_rsp_data.actuator_types, 
                   0x0000,
                   sizeof(model_rsp_data.actuator_types));

            if(p_event->firsttype <= sensor_type_internal_air_temperature)
            {
                model_rsp_data.actuator_types.types[0] = 
                                        sensor_type_internal_air_temperature;
                model_rsp_data.actuator_types.types[1] = 
                                        sensor_type_desired_air_temperature;
                model_rsp_data.actuator_types.types_len = 2;
            }
            else if(p_event->firsttype <= sensor_type_desired_air_temperature)
            {
                model_rsp_data.actuator_types.types[0] = 
                                        sensor_type_desired_air_temperature;
                model_rsp_data.actuator_types.types_len = 1;
            }
            else
            {
                model_rsp_data.actuator_types.types[0] = sensor_type_invalid;
                model_rsp_data.actuator_types.types_len = 0;
            }

            model_rsp_data.actuator_types.tid = p_event->tid;
            /* Send response data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&model_rsp_data.actuator_types;
            }
        }
        break;


        case CSRMESH_ACTUATOR_SET_VALUE_NO_ACK:
        case CSRMESH_ACTUATOR_SET_VALUE:
        {
            CSRMESH_ACTUATOR_SET_VALUE_T *p_event = 
                                    (CSRMESH_ACTUATOR_SET_VALUE_T *)data->data;

            if(p_event->type == sensor_type_desired_air_temperature)
            {
                /* Length of the value of this type is 16 bits */
                current_desired_air_temp = p_event->value[0] + 
                                          (p_event->value[1] << 8);

                DEBUG_STR(" RECEIVED DESIRED TEMP : ");
                PrintInDecimal(current_desired_air_temp/32);
                DEBUG_STR(" kelvin\r\n");

                /* Duplicate messages could be filtered here */
                if(current_desired_air_temp != last_bcast_desired_air_temp)
                {
                    /* Set last Broadcast Temperature to current temperature.*/
                    last_bcast_desired_air_temp = current_desired_air_temp;
                    StartTempTransmission();
                }

                if(event_code == CSRMESH_ACTUATOR_SET_VALUE)
                {
                    model_rsp_data.actuator_value.type = p_event->type;
                    model_rsp_data.actuator_value.tid = p_event->tid;

                    /* Send response data to model */
                    if (state_data != NULL)
                    {
                        *state_data = (void *)&model_rsp_data.actuator_value;
                    }
                }
                else
                {
                    *state_data = NULL;
                }
            }
            else
            {
                *state_data = NULL;
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
    /* Due to multiple retransmissions it sometimes happens that the device
     * receives its own message relayed by other devices. So check the source
     * ID and discard the message if it is our own message
     */
    uint16 self_dev_id = 0;
    CSR_MESH_APP_EVENT_DATA_T get_dev_id_data;

    get_dev_id_data.appCallbackDataPtr = &self_dev_id;
    CSRmeshGetDeviceID(CSR_MESH_DEFAULT_NETID, &get_dev_id_data);
    if(self_dev_id == data->src_id)
    {
        *state_data = NULL;
        return CSR_MESH_RESULT_SUCCESS;
    }

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
                model_rsp_data.sensor_types.types[0] = sensor_type_invalid;
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

        case CSRMESH_SENSOR_READ_VALUE:
        {
            CSRMESH_SENSOR_READ_VALUE_T *p_event = 
                                    (CSRMESH_SENSOR_READ_VALUE_T *)data->data;
            MemSet(&model_rsp_data.sensor_value,
                   0x0000,
                   sizeof(model_rsp_data.sensor_value));
            bool send_ack = FALSE;

            if(p_event->type == sensor_type_internal_air_temperature)
            {
                model_rsp_data.sensor_value.type = 
                                        sensor_type_internal_air_temperature;
                CsrUint8 *p_temp = model_rsp_data.sensor_value.value;
                BufWriteUint16(&p_temp, current_air_temp);
                model_rsp_data.sensor_value.value_len = 2;
                send_ack = TRUE;
            }
            else if(p_event->type == sensor_type_desired_air_temperature)
            {
                model_rsp_data.sensor_value.type = 
                                        sensor_type_desired_air_temperature;
                CsrUint8 *p_temp = model_rsp_data.sensor_value.value;
                BufWriteUint16(&p_temp, current_desired_air_temp);
                model_rsp_data.sensor_value.value_len = 2;
                send_ack = TRUE;
            }

            if(p_event->type2 == sensor_type_desired_air_temperature)
            {
                model_rsp_data.sensor_value.type2 = 
                                        sensor_type_desired_air_temperature;
                CsrUint8 *p_temp = model_rsp_data.sensor_value.value2;
                BufWriteUint16(&p_temp, current_desired_air_temp);
                model_rsp_data.sensor_value.value2_len = 2;
                send_ack = TRUE;
            }
            else if(p_event->type2 == sensor_type_internal_air_temperature)
            {
                model_rsp_data.sensor_value.type2 = 
                                        sensor_type_internal_air_temperature;
                CsrUint8 *p_temp = model_rsp_data.sensor_value.value2;
                BufWriteUint16(&p_temp, current_air_temp);
                model_rsp_data.sensor_value.value2_len = 2;
                send_ack = TRUE;
            }

            if(send_ack == TRUE)
            {
                /* Start the current temperature transmission as it would  */
                StartTempTransmission();

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
        break;

        case CSRMESH_SENSOR_MISSING:
        {
            CSRMESH_SENSOR_MISSING_T *p_event = 
                                         (CSRMESH_SENSOR_MISSING_T *)data->data;
            uint8 index = 0;
            uint8* types;
            uint16 type;
            bool send_ack = FALSE, first_val_inserted = FALSE;

            MemSet(&model_rsp_data.sensor_value, 
                   0x0000,
                   sizeof(model_rsp_data.sensor_value));

            types =(uint8 *) p_event->types;
            for(index = 0; index < 8; index= index+2)
            {
                type = (uint16)BufReadUint16(&types);
                if(type == sensor_type_internal_air_temperature)
                {
                    DEBUG_STR(" MISSING current temp");
                    if(first_val_inserted == FALSE)
                    {
                        model_rsp_data.sensor_value.type = 
                                        sensor_type_internal_air_temperature;
                        CsrUint8 *p_temp = model_rsp_data.sensor_value.value;
                        BufWriteUint16(&p_temp, current_air_temp);
                        model_rsp_data.sensor_value.value_len = 2;
                    }
                    else
                    {
                        model_rsp_data.sensor_value.type2 = 
                                        sensor_type_internal_air_temperature;
                        CsrUint8 *p_temp = model_rsp_data.sensor_value.value2;
                        BufWriteUint16(&p_temp, current_air_temp);
                        model_rsp_data.sensor_value.value2_len = 2;
                        break;
                    }
                    send_ack = TRUE;
                }
                else if(type == sensor_type_desired_air_temperature)
                {
                    DEBUG_STR(" MISSING desired air temp");
                    if(first_val_inserted == FALSE)
                    {
                        model_rsp_data.sensor_value.type = 
                                        sensor_type_desired_air_temperature;
                        CsrUint8 *p_temp = model_rsp_data.sensor_value.value;
                        BufWriteUint16(&p_temp, current_desired_air_temp);
                        model_rsp_data.sensor_value.value_len = 2;
                    }
                    else
                    {
                        model_rsp_data.sensor_value.type2 = 
                                        sensor_type_desired_air_temperature;
                        CsrUint8 *p_temp = model_rsp_data.sensor_value.value2;
                        BufWriteUint16(&p_temp, current_desired_air_temp);
                        model_rsp_data.sensor_value.value2_len = 2;
                        break;
                    }
                    send_ack = TRUE;
                }
            }
            if(send_ack == TRUE)
            {
                /* Start the current temperature transmission as it would  */
                StartTempTransmission();

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
        break;

        case CSRMESH_SENSOR_SET_STATE:
        {
            CSRMESH_SENSOR_SET_STATE_T *p_event = 
                                    (CSRMESH_SENSOR_SET_STATE_T *)data->data;
            bool send_ack = FALSE;

            /* repeat interval of desired or current temp has changed */
            if(p_event->type == sensor_type_desired_air_temperature)
            {
                sensor_data[DESIRED_AIR_TEMP_IDX].repeat_interval = 
                                                        p_event->repeatinterval;
                WriteSensorDataToNVM(DESIRED_AIR_TEMP_IDX);
                send_ack = TRUE;
            }
            else if(p_event->type == sensor_type_internal_air_temperature)
            {
                sensor_data[CURRENT_AIR_TEMP_IDX].repeat_interval = 
                                                        p_event->repeatinterval;
                WriteSensorDataToNVM(CURRENT_AIR_TEMP_IDX); 
                send_ack = TRUE;
            }

            if(send_ack == TRUE)
            {
                g_tsapp_data.sensor_model.type = p_event->type;
                g_tsapp_data.sensor_model.repeatinterval = 
                                                    p_event->repeatinterval;
                g_tsapp_data.sensor_model.tid = p_event->tid;
 
                /* Send response data to model */
                if (state_data != NULL)
                {
                    *state_data = (void *)&g_tsapp_data.sensor_model;
                }

                /* As repeat interval might have changed restart or stop the 
                 * timer as per the new interval value.
                 */
                if(sensor_data[CURRENT_AIR_TEMP_IDX].repeat_interval != 0 ||
                   sensor_data[DESIRED_AIR_TEMP_IDX].repeat_interval != 0)
                {
                    startRepeatIntervalTimer();
                }
                else
                {
                    /* Delete the repeat interval timer */
                    TimerDelete(repeat_interval_tid);
                    repeat_interval_tid = TIMER_INVALID;
                }
            }
            else
            {
                *state_data = NULL;
            }
        }
        break;

        case CSRMESH_SENSOR_GET_STATE:
        {
            CSRMESH_SENSOR_GET_STATE_T *p_event = 
                                    (CSRMESH_SENSOR_GET_STATE_T *)data->data;
            bool send_ack = FALSE;

            if(p_event->type == sensor_type_desired_air_temperature)
            {
                g_tsapp_data.sensor_model.repeatinterval = 
                            sensor_data[DESIRED_AIR_TEMP_IDX].repeat_interval;
                send_ack = TRUE;
            }
            else if(p_event->type == sensor_type_internal_air_temperature)
            {
                g_tsapp_data.sensor_model.repeatinterval = 
                            sensor_data[CURRENT_AIR_TEMP_IDX].repeat_interval;
                send_ack = TRUE;
            }

            if(send_ack == TRUE)
            {
                g_tsapp_data.sensor_model.type = p_event->type;
                g_tsapp_data.sensor_model.tid = p_event->tid;
 
                /* Send response data to model */
                if (state_data != NULL)
                {
                    *state_data = (void *)&g_tsapp_data.sensor_model;
                }
            }
            else
            {
                *state_data = NULL;
            }
        }
        break;

        case CSRMESH_SENSOR_VALUE:
        {
#ifdef ENABLE_ACK_MODE
            CSRMESH_SENSOR_VALUE_T *p_event = 
                                    (CSRMESH_SENSOR_VALUE_T *)data->data;

            SENSOR_FORMAT_TEMPERATURE_T desired_air_temp_recvd = 0;
            SENSOR_FORMAT_TEMPERATURE_T current_air_temp_recvd = 0;
            uint8 *value, *value2;

            value = p_event->value;
            if(p_event->type == sensor_type_desired_air_temperature) 
            {
                desired_air_temp_recvd = (uint16)BufReadUint16(&value);
            }
            else if(p_event->type == sensor_type_internal_air_temperature)
            {
                current_air_temp_recvd = (uint16)BufReadUint16(&value);
            }

            value2 = p_event->value2;
            if(p_event->type2 == sensor_type_desired_air_temperature) 
            {
                desired_air_temp_recvd = (uint16)BufReadUint16(&value2);
            }
            else if(p_event->type2 == sensor_type_internal_air_temperature)
            {
                current_air_temp_recvd = (uint16)BufReadUint16(&value2);
            }

            /* We have received acknowledgement for the write_value sent.
             * update the acknowlege heater device list.
             */
            if(current_air_temp_recvd == last_bcast_air_temp &&
               desired_air_temp_recvd == last_bcast_desired_air_temp)
            {
                uint16 index;
                bool dev_found = FALSE;
                uint16 src_id = data->src_id;

                /* Check whether the heater is present in the list if its 
                 * present then update the ack recvd from the heater,
                 * otherwise add the new heater onto the list and increase 
                 * the count.
                 */
                 for(index = 0; index < MAX_HEATERS_IN_GROUP; index++)
                 {
                    if(heater_list[index].dev_id == src_id)
                    {
                        heater_list[index].ack_recvd = TRUE;
                        dev_found = TRUE;
                        heater_list[index].no_response_count = 0;
                        break;
                    }
                 }
                 if(dev_found == FALSE)
                 {
                    for(index = 0; index < MAX_HEATERS_IN_GROUP; index++)
                    {
                        if(heater_list[index].dev_id == MESH_BROADCAST_ID)
                        {
                            heater_list[index].dev_id = src_id;
                            heater_list[index].ack_recvd = TRUE;
                            heater_list[index].no_response_count = 0;
                            break;
                        }
                    }
                 }
            }
#endif /* ENABLE_ACK_MODE */
        }
        break;

        case CSRMESH_SENSOR_WRITE_VALUE:
        case CSRMESH_SENSOR_WRITE_VALUE_NO_ACK:
        {
            CSRMESH_SENSOR_WRITE_VALUE_T *p_event = 
                                    (CSRMESH_SENSOR_WRITE_VALUE_T *)data->data;
            bool send_ack = FALSE;

            MemSet(&model_rsp_data.sensor_value, 
                   0x0000,
                   sizeof(model_rsp_data.sensor_value));

            if(p_event->type == sensor_type_desired_air_temperature) 
            {
                current_desired_air_temp = (uint16)p_event->value[0] + 
                                                   (p_event->value[1] << 8);
                model_rsp_data.sensor_value.type = p_event->type;
                model_rsp_data.sensor_value.value[0] = p_event->value[0];
                model_rsp_data.sensor_value.value[1] = p_event->value[1];
                model_rsp_data.sensor_value.value_len = 2;
                send_ack = TRUE;
            }
            else if(p_event->type == sensor_type_internal_air_temperature)
            {
                CsrUint8 *p_temp = model_rsp_data.sensor_value.value;
                model_rsp_data.sensor_value.type = p_event->type;

                BufWriteUint16(&p_temp, current_air_temp);
                model_rsp_data.sensor_value.value_len = 2;
                send_ack = TRUE;
            }

            if(p_event->type2 == sensor_type_desired_air_temperature)
            {
                uint8 *value = p_event->value2;
                CsrUint8 *p_temp = model_rsp_data.sensor_value.value2;

                current_desired_air_temp = (uint16)BufReadUint16(&value);

                model_rsp_data.sensor_value.type2 = p_event->type2;
                BufWriteUint16(&p_temp, current_desired_air_temp);
                model_rsp_data.sensor_value.value2_len = 2;
                send_ack = TRUE;
            }

            /* Duplicate messages could be filtered here */
            if(current_desired_air_temp != last_bcast_desired_air_temp)
            {
                /* Set last Broadcast Temperature to current temperature. */
                last_bcast_desired_air_temp = current_desired_air_temp;
                StartTempTransmission();
            }

            if(event_code == CSRMESH_SENSOR_WRITE_VALUE && send_ack == TRUE)
            {
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

            g_tsapp_data.attn_model.attractattention = p_event->attractattention;
            g_tsapp_data.attn_model.duration = p_event->duration;
            g_tsapp_data.attn_model.tid = p_event->tid;

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
                    attn_tid = TimerCreate((uint32)p_event->duration * MILLISECOND,
                                                        TRUE, attnTimerHandler);
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
                *state_data = (void *)&g_tsapp_data.attn_model;
            }

        }
        break;
        
        default:
        break;
    }

    return CSR_MESH_RESULT_SUCCESS;
}

#ifdef ENABLE_BATTERY_MODEL
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
            g_tsapp_data.battery_model.batterylevel = ReadBatteryLevel();
            g_tsapp_data.battery_model.batterystate = GetBatteryState();
            g_tsapp_data.battery_model.tid = p_get_state->tid;

            /* Pass Battery state data to model */
            if (state_data != NULL)
            {
                *state_data = (void *)&g_tsapp_data.battery_model;
            }
        }
        break;

        default:
        break;
    }
    
    return CSR_MESH_RESULT_SUCCESS;
}

#endif /* ENABLE_BATTERY_MODEL */


/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 * FILE
 *      user_config.h
 *
 * DESCRIPTION
 *      This file contains definitions which will enable customization of the
 *      application.
 *
 ******************************************************************************/

#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

/*=============================================================================*
 *  Public Definitions
 *============================================================================*/
/* Application version */
#define APP_MAJOR_VERSION       (2)
#define APP_MINOR_VERSION       (0)

/* Application NVM version. This version is used to keep the compatibility of
 * NVM contents with the application version. This value needs to be modified
 * only if the new version of the application has a different NVM structure
 * than the previous version (such as number of groups supported) that can
 * shift the offsets of the currently stored parameters.
 * If the application NVM version has changed, it could still read the values
 * from the old Offsets and store into new offsets.
 * This application currently erases all the NVM values if the NVM version has
 * changed.
 */
#define APP_NVM_VERSION         (1)

#define CSR_MESH_SENSOR_PID     (0x1062)

/* Vendor ID for CSR */
#define APP_VENDOR_ID           (0x0A12)

/* Product ID. */
#define APP_PRODUCT_ID          (CSR_MESH_SENSOR_PID)

/* Version Number. */
#define APP_VERSION             (((uint32)(APP_PRODUCT_ID    & 0xFF) << 24) | \
                                ((uint32)(APP_NVM_VERSION   & 0xFF) << 16) | \
                                ((uint32)(APP_MAJOR_VERSION & 0xFF) << 8)  | \
                                ((uint32)(APP_MINOR_VERSION & 0xFF)))

/* Number of sensor model groups. */
#define NUM_SENSOR_MODEL_GROUPS  (4)

/* Number of attention model groups. */
#define NUM_ATT_MODEL_GROUPS     (4)

/* Number of data model groups */
#define NUM_DATA_MODEL_GROUPS    (4)

/* Enable Static Random Address. */
/* #define USE_STATIC_RANDOM_ADDRESS */

/* Enable battery model support */
/* #define ENABLE_BATTERY_MODEL */

/* Macro to enable Data Model support 
#define ENABLE_DATA_MODEL */

/* STTS751 Temperature Sensor. */
#define TEMPERATURE_SENSOR_STTS751

/* Enable application debug logging on UART */
#define DEBUG_ENABLE 

/* Temperature Sensor Parameters. */
/* Temperature Sensor Sampling Interval */
#define TEMPERATURE_SAMPLING_INTERVAL  (15 * SECOND)

/* Temperature Controller. */
#define ENABLE_TEMPERATURE_CONTROLLER

/* Enable the Acknowledge mode */
/* #define ENABLE_ACK_MODE */

/* Default repeat interval in seconds. This enables the sensor periodically
 * sending the temperature every repeat interval. Value range 0-255. The
 * interval within 1-30 seconds is considered to be min of 30 due to the 
 * retransmissions.
 */
#define DEFAULT_REPEAT_INTERVAL        (0)

/* Default rx duty cycle.in percentage */
#define DEFAULT_RX_DUTY_CYCLE          (2)

/* Rx duty cycle in percentage when device set to active scan mode. The device  
 * is present in this mode before grouping and on attention or data stream in 
 * progress.
 */
#define HIGH_RX_DUTY_CYCLE             (100)

/* Number of msgs to be added in the transmit queue in one shot.
 * supported values 1-5
 */
#define TRANSMIT_MSG_DENSITY           (2)

/* Number of msgs to be retransmitted per temp change */
#define NUM_OF_RETRANSMISSIONS         (60)

/* Temperature change in 1/32 kelvin units. If temp changes more than this the
 * sensor would write the temp change onto the group.
 */
#define TEMPERATURE_CHANGE_TOLERANCE   (32)

/* Enable the this definition to use an authorisation code for association */
#define USE_AUTHORISATION_CODE 

/* Enable Device UUID Advertisements
#define ENABLE_DEVICE_UUID_ADVERTS */

#endif /* __USER_CONFIG_H__ */


/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
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

#define NUM_SENSOR_MODEL_GROUPS  (4)

/* Number of attention model groups. */
#define NUM_ATT_MODEL_GROUPS     (4)

/* Number of data model groups */
#define NUM_DATA_MODEL_GROUPS    (4)

/* Enable battery model support */
/* #define ENABLE_BATTERY_MODEL */

/* Macro to enable Data Model support */
#define ENABLE_DATA_MODEL

/* Default rx duty cycle in percentage */
#define DEFAULT_RX_DUTY_CYCLE          (2)

/* Rx duty cycle in percentage when device set to active scan mode. The device  
 * is present in this mode before grouping and on attention or data stream in 
 * progress.
 */
#define HIGH_RX_DUTY_CYCLE             (100)

/* Maximum time the message should be retransmitted */
#define MAX_RETRANSMISSION_TIME        (7500 * MILLISECOND)

/* Message Retransmit timer in milliseconds */
#define RETRANSMIT_INTERVAL            (500 * MILLISECOND)

/* Enable the Acknowledge mode */
/* #define ENABLE_ACK_MODE */

/* Enable Static Random Address. */
/* #define USE_STATIC_RANDOM_ADDRESS */

/* Enable application debug logging on UART */
#define DEBUG_ENABLE

/* Enable Device UUID Advertisements 
#define ENABLE_DEVICE_UUID_ADVERTS */

/* Enable the this definition to use an authorisation code for association */
#define USE_AUTHORISATION_CODE 

#endif /* __USER_CONFIG_H__ */


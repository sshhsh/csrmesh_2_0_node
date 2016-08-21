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
#define APP_MAJOR_VERSION   (2)
#define APP_MINOR_VERSION   (0)

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
#define APP_NVM_VERSION     (1)

#define CSR_MESH_SWITCH_PID (0x1061)

/* Vendor ID for CSR */
#define APP_VENDOR_ID       (0x0A12)

/* Product ID. */
#define APP_PRODUCT_ID      (CSR_MESH_SWITCH_PID)

/* Number of model groups supported 
 * The switch application uses the Group ID assigned to the switch model as
 * the destination address for sending light control messages.
 * Since we only have one set of buttons(brightness control) and a switch(power 
 * control) on the IOT lighting board, to control destination devices, we
 * support only one group for all the supported models.
 */
#define MAX_MODEL_GROUPS     (1)

/* Version Number. */
#define APP_VERSION         (((uint32)(APP_PRODUCT_ID    & 0xFF) << 24) | \
                             ((uint32)(APP_NVM_VERSION   & 0xFF) << 16) | \
                             ((uint32)(APP_MAJOR_VERSION & 0xFF) << 8)  | \
                             ((uint32)(APP_MINOR_VERSION & 0xFF)))

/* Enable Static Random Address. */
/* #define USE_STATIC_RANDOM_ADDRESS */

/* Enable Watchdog Model. */
#define ENABLE_WATCHDOG_MODEL

/* Enable Data Model */
#define ENABLE_DATA_MODEL

/* Enable application debug logging on UART */
/* #define DEBUG_ENABLE */

/* Default rx duty cycle in percentage */
#define DEFAULT_RX_DUTY_CYCLE          (5)

/* Rx duty cycle in percentage when device set to active scan mode. The device  
 * is present in this mode before association and on attention, data stream in 
 * progress or during when watchdog is enabled.
 */
#define HIGH_RX_DUTY_CYCLE             (100)

/* Enable the this definition to use an authorisation code for association */
#define USE_AUTHORISATION_CODE 

/* Enable Device UUID Advertisements 
#define ENABLE_DEVICE_UUID_ADVERTS */

#endif /* __USER_CONFIG_H__ */


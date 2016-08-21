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

#define CSR_MESH_LIGHT_PID      (0x1060)

/* Vendor ID for CSR */
#define APP_VENDOR_ID           (0x0A12)

/* Product ID. */
#define APP_PRODUCT_ID          (CSR_MESH_LIGHT_PID)

/* Number of model groups supported */
#define MAX_MODEL_GROUPS        (4)

/* Version Number. */
#define APP_VERSION             (((uint32)(APP_PRODUCT_ID    & 0xFF) << 24) | \
                                 ((uint32)(APP_NVM_VERSION   & 0xFF) << 16) | \
                                 ((uint32)(APP_MAJOR_VERSION & 0xFF) << 8)  | \
                                 ((uint32)(APP_MINOR_VERSION & 0xFF)))

/* Enable fast PWM using PIO controller instead of Hardware PWM */
#define ENABLE_FAST_PWM

/* Enable support for setting the color temperature */
/*#define COLOUR_TEMP_ENABLED*/

/* Enable application debug logging on UART */
/*#define DEBUG_ENABLE*/

#if !defined(DEBUG_ENABLE)
#define USE_ASSOCIATION_REMOVAL_KEY

/* Association Removal Button Press Duration */
#define LONG_KEYPRESS_TIME      (2 * SECOND)
#endif

/* Enable Static Random Address for bridge connectable advertisements */
/* #define USE_STATIC_RANDOM_ADDRESS */

/* Enable Data model support 
 * Light, Power, attention and battery models are always enabled
 */
#define ENABLE_DATA_MODEL

/* Enable the this definition to use an authorisation code for association */
/*#define USE_AUTHORISATION_CODE */

/* Enable Device UUID Advertisements */
#define ENABLE_DEVICE_UUID_ADVERTS

#endif /* __USER_CONFIG_H__ */


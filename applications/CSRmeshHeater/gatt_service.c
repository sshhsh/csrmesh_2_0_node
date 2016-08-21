/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *    gatt_service.c
 *
 *  DESCRIPTION
 *    This file defines routines for using GATT service.
 *
 *****************************************************************************/

/*=============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <gatt.h>
#include <gatt_uuid.h>
#include <buf_utils.h>
#include <nvm.h>

/*=============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "ota_customisation.h"

#include "gatt_service.h"

/*============================================================================*
 *  Private Definitions
 *============================================================================*/


/*=============================================================================*
 *  Private Data Types
 *============================================================================*/

/* GATT service data structure */
typedef struct
{
    /* The current configuration for the Service Changed characteristic */
    gatt_client_config serv_changed_config;
} GATT_DATA_T;

/*=============================================================================*
 *  Private Data
 *============================================================================*/

/* GATT service data instance */
static GATT_DATA_T g_gatt_data;

/*=============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattDataInit
 *
 *  DESCRIPTION
 *      This function is used to initialise gatt service data 
 *      structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void GattDataInit(void)
{
    /* Initialise Service Changed Client Configuration Characterisitic
     * descriptor value only if device is not bonded
     */
    g_gatt_data.serv_changed_config = gatt_client_config_none;
}



/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattServiceChangedIndActive
 *
 *  DESCRIPTION
 *      This function allows other modules to read whether the bonded device
 *      has requested indications on the Service Changed characteristic.
 *
 *  RETURNS
 *      True if indications are requested, false otherwise.
 *
 *----------------------------------------------------------------------------*/

extern bool GattServiceChangedIndActive(void)
{
    return (IS_PAIRED && 
            (g_gatt_data.serv_changed_config == gatt_client_config_indication));
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles READ operations on GATT service attributes.
 *      It responds with the GATT_ACCESS_RSP message.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void GattHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    uint16  data_length = 0;
    uint8   value[2]; 
    uint8   *p_value = NULL;
    sys_status rc = gatt_status_read_not_permitted;

    if(p_ind->handle == HANDLE_SERVICE_CHANGED_CLIENT_CONFIG)
    {
        /* Service changed client characteristic configuration descriptor read
         * has been requested
         */
        data_length = 2;
        p_value = value;
        BufWriteUint16((uint8 **)&p_value, g_gatt_data.serv_changed_config);
        rc = sys_status_success;
    }

    /* Send Access Response */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, data_length, value);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattHandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles WRITE operations on GATT service attributes.
 *      It responds with the GATT_ACCESS_RSP message.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void GattHandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    uint16  client_config;
    uint8  *p_value = p_ind->value;
    sys_status rc = gatt_status_write_not_permitted;
    
    if(p_ind->handle == HANDLE_SERVICE_CHANGED_CLIENT_CONFIG)
    {
        client_config = BufReadUint16(&p_value);
        
        /* Client configuration is a bit field value, so ideally bit wise
         * comparison should be used but since the application supports only
         * indications or nothing, direct comparison should be used.
         */
        if((client_config == gatt_client_config_indication) ||
           (client_config == gatt_client_config_none))
        {
            g_gatt_data.serv_changed_config = client_config;
            rc = sys_status_success;
        }
        else
        {
            rc = gatt_status_desc_improper_config;
        }
    }
    /* Send Access Response */
    GattAccessRsp(p_ind->cid, p_ind->handle, rc, 0, NULL);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      GattCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the GATT 
 *      service
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern bool GattCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_GATT_SERVICE) &&
            (handle <= HANDLE_GATT_SERVICE_END))
            ? TRUE : FALSE;
}


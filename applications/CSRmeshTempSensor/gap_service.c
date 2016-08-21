/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      gap_service.c
 *
 *  DESCRIPTION
 *      This file defines routines for using GAP service.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <gatt.h>
#include <gatt_prim.h>
#include <mem.h>
#include <buf_utils.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/

#include "app_gatt.h"
#include "gap_service.h"
#include "app_gatt_db.h"
#include "nvm_access.h"

/*============================================================================*
 *  Private Data Types
 *============================================================================*/

/* GAP service data type */
typedef struct
{
    /* Name length in Bytes */
    uint16  length;

    /* Pointer to hold device name used by the application */
    uint8   *p_dev_name;

    /* NVM offset at which GAP service data is stored */
    uint16  nvm_offset;

} GAP_DATA_T;

typedef struct
{
    uint8 ad_type;
    uint8 name[DEVICE_NAME_MAX_LENGTH + 2];
}GAP_DEVICE_NAME_T;

/*============================================================================*
 *  Private Data
 *============================================================================*/

/* GAP service data instance */
static GAP_DATA_T g_gap_data;

/* Default device name - Added two for storing AD Type and Null ('\0') */ 
GAP_DEVICE_NAME_T g_device_name = {AD_TYPE_LOCAL_NAME_COMPLETE,
                                   "CSRmesh TSensor"};

/*============================================================================*
 *  Private Definitions
 *============================================================================*/

/* Number of words of NVM memory used by GAP service */

/* Add space for Device Name Length and Device Name */
#define GAP_SERVICE_NVM_MEMORY_WORDS  (1 + DEVICE_NAME_MAX_LENGTH)

/* The offset of data being stored in NVM for GAP service. This offset is 
 * added to GAP service offset to NVM region (see g_gap_data.nvm_offset) 
 * to get the absolute offset at which this data is stored in NVM
 */
#define GAP_NVM_DEVICE_LENGTH_OFFSET  (0)

#define GAP_NVM_DEVICE_NAME_OFFSET    (1)

/*============================================================================*
 *  Private Function Prototypes
 *============================================================================*/

static void updateDeviceName(uint16 length, uint8 *name);

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      gapWriteDeviceNameToNvm
 *
 *  DESCRIPTION
 *      This function is used to write GAP Device Name Length and Device Name 
 *      to NVM 
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

static void gapWriteDeviceNameToNvm(void)
{

    /* Write device name length to NVM */
     Nvm_Write(&g_gap_data.length, sizeof(g_gap_data.length), 
              g_gap_data.nvm_offset + 
              GAP_NVM_DEVICE_LENGTH_OFFSET); 

    /* Write device name to NVM 
     * Typecast of uint8 to uint16 or vice-versa shall not have any side 
     * affects as both types (uint8 and uint16) take one word memory on XAP
     */
     Nvm_Write((uint16*)g_gap_data.p_dev_name, g_gap_data.length, 
              g_gap_data.nvm_offset + 
              GAP_NVM_DEVICE_NAME_OFFSET); 

}


/*----------------------------------------------------------------------------*
 *  NAME
 *      updateDeviceName
 *
 *  DESCRIPTION
 *      This function updates the device name and length in gap service.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

static void updateDeviceName(uint16 length, uint8 *name)
{
    uint8   *p_name = g_gap_data.p_dev_name;
    
    /* Update Device Name length to the maximum of DEVICE_NAME_MAX_LENGTH  */
    if(length < DEVICE_NAME_MAX_LENGTH)
        g_gap_data.length = length;
    else
        g_gap_data.length = DEVICE_NAME_MAX_LENGTH;

    MemCopy(p_name, name, g_gap_data.length);

    /* Null terminate the device name string */
    p_name[g_gap_data.length] = '\0';

    gapWriteDeviceNameToNvm();

}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME 
 *      GapDataInit
 *
 *  DESCRIPTION
 *      This function is used to initialise GAP service data 
 *      structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void GapDataInit(void)
{
    /* Skip 1st byte to move over AD Type field and point to device name */
    g_gap_data.p_dev_name = (uint8 *)(g_device_name.name);
    g_gap_data.length = StrLen((char *)g_gap_data.p_dev_name);
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      GapHandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operation on GAP service attributes
 *      maintained by the application and responds with the GATT_ACCESS_RSP 
 *      message.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void GapHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    uint16 length = 0;
    uint8  *p_value = NULL;
    sys_status rc = sys_status_success;

    switch(p_ind->handle)
    {

        case HANDLE_DEVICE_NAME:
        {
            /* Validate offset against length, it should be less than 
             * device name length
             */
            if(p_ind -> offset < g_gap_data.length)
            {
                length = g_gap_data.length - p_ind -> offset;
                p_value = (g_gap_data.p_dev_name + p_ind -> offset);
            }
            else
            {
                rc = gatt_status_invalid_offset;
            }
        }
        break;

        default:
            /* No more IRQ characteristics */
            rc = gatt_status_read_not_permitted;
        break;

    }

    GattAccessRsp(p_ind->cid, p_ind->handle, rc,
                  length, p_value);

}


/*----------------------------------------------------------------------------*
 *  NAME
 *      GapHandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles write operation on GAP service attributes
 *      maintained by the application and responds with the GATT_ACCESS_RSP 
 *      message.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void GapHandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    sys_status rc = sys_status_success;

    switch(p_ind->handle)
    {

        case HANDLE_DEVICE_NAME:
            /* Update device name */
            updateDeviceName(p_ind->size_value, p_ind->value);
        break;

        default:
            /* No more IRQ characteristics */
            rc = gatt_status_write_not_permitted;
        break;

    }

    GattAccessRsp(p_ind->cid, p_ind->handle, rc, 0, NULL);

}


/*----------------------------------------------------------------------------*
 *  NAME
 *      GapReadDataFromNVM
 *
 *  DESCRIPTION
 *      This function is used to read GAP specific data stored in NVM
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void GapReadDataFromNVM(uint16 *p_offset)
{
    g_gap_data.nvm_offset = *p_offset;

    /* Read Device Length */
    Nvm_Read(&g_gap_data.length, sizeof(g_gap_data.length), 
             *p_offset + 
             GAP_NVM_DEVICE_LENGTH_OFFSET);

    /* If the dev name length read from the NVM is incorrect then better update
     * the NVM with the correct device name and its length.
     */
    if(g_gap_data.length > DEVICE_NAME_MAX_LENGTH)
    {
        g_gap_data.p_dev_name = (uint8 *)(g_device_name.name);
        g_gap_data.length = StrLen((char *)g_gap_data.p_dev_name);
        updateDeviceName(g_gap_data.length, g_gap_data.p_dev_name);
    }
    else
    {
    /* Read Device Name 
     * Typecast of uint8 to uint16 or vice-versa shall not have any side 
     * affects as both types (uint8 and uint16) take one word memory on XAP
     */
    Nvm_Read((uint16*)g_gap_data.p_dev_name, g_gap_data.length, 
             *p_offset + 
             GAP_NVM_DEVICE_NAME_OFFSET);

    /* Add NULL character to terminate the device name string */
    g_gap_data.p_dev_name[g_gap_data.length] = '\0';
    }
    /* Increase NVM offset for maximum device name length. Add 1 for
     * 'device name length' field as well
     */
    *p_offset += DEVICE_NAME_MAX_LENGTH + 1;

}


/*----------------------------------------------------------------------------*
 *  NAME
 *      GapInitWriteDataToNVM
 *
 *  DESCRIPTION
 *      This function is used to write GAP specific data to NVM for 
 *      the first time during application initialisation
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void GapInitWriteDataToNVM(uint16 *p_offset)
{

    /* The NVM offset at which GAP service data will be stored */
    g_gap_data.nvm_offset = *p_offset;

    gapWriteDeviceNameToNvm();

    /* Increase NVM offset for maximum device name length. Add 1 for
     * 'device name length' field as well
     */
    *p_offset += DEVICE_NAME_MAX_LENGTH + 1;

}


/*----------------------------------------------------------------------------*
 *  NAME
 *      GapCheckHandleRange
 *
 *  DESCRIPTION
 *      This function is used to check if the handle belongs to the GAP 
 *      service
 *
 *  RETURNS
 *      Boolean - Indicating whether handle falls in range or not.
 *
 *---------------------------------------------------------------------------*/

extern bool GapCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_GAP_SERVICE) &&
            (handle <= HANDLE_GAP_SERVICE_END))
            ? TRUE : FALSE;
}


/*----------------------------------------------------------------------------*
 *  NAME
 *      GapGetNameAndLength
 *
 *  DESCRIPTION
 *      This function is used to get the reference to the 'g_device_name' array, 
 *      which contains AD Type and device name. This function also returns the 
 *      AD Type and device name length.
 *
 *  RETURNS
 *      Pointer to device name array.
 *
 *---------------------------------------------------------------------------*/

extern uint8 *GapGetNameAndLength(uint16 *p_name_length)
{
    *p_name_length = StrLen((char *)g_device_name.name)+1;
    return ((uint8 *)&g_device_name);
}

#ifdef NVM_TYPE_FLASH
/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteGapServiceDataInNVM
 *
 *  DESCRIPTION
 *      This function writes the GAP service data in NVM
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/

extern void WriteGapServiceDataInNVM(void)
{
    /* Gap Service has only device name to write into NVM */
    gapWriteDeviceNameToNvm();
}
#endif /* NVM_TYPE_FLASH */


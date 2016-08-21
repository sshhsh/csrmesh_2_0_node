/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      csr_ota_service.c
 *
 *  DESCRIPTION
 *      Implementation of the CSR OTA Update Application Service.
 *
 *****************************************************************************/

/*=============================================================================*
 *  SDK Header Files
 *============================================================================*/

#include <gatt.h>
#include <gatt_uuid.h>
#include <buf_utils.h>
#include <nvm.h>
#include <ble_hci_test.h>
#include <csr_ota.h>
#include <memory.h>
#include <mem.h>

/*=============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"
#include "ota_customisation.h"

#include "app_gatt.h"

#if defined(USE_STATIC_RANDOM_ADDRESS)
#include <gap_app_if.h>
#endif /* USE_STATIC_RANDOM_ADDRESS */

#include "gatt_service.h"
#include "csr_ota_service.h"
#include "csr_ota_uuids.h"

/*============================================================================*
 *  Private Data Types
 *============================================================================*/

/* OTA Service data type */
typedef struct
{
    /* Flag for OTA reset */
    bool                    ota_reset_required;

} OTA_SERV_DATA_T;

/*=============================================================================*
 *  Private Data
 *============================================================================*/

/* The current value of the DATA TRANSFER characteristic */
static uint8 data_transfer_memory[MAX_DATA_LENGTH] = {0};

/* The number of bytes of valid data in data_transfer_memory */
static uint8 data_transfer_data_length = 0;

/* The current configuration of the DATA TRANSFER characteristic */
static uint8 data_transfer_configuration[2] = {gatt_client_config_none, 0};

/* OTA service data structure */
static OTA_SERV_DATA_T ota_serv_data;

/*=============================================================================*
 *  Private Function Implementations
 *============================================================================*/
/*-----------------------------------------------------------------------------*
 *  NAME
 *      readCsBlock
 *
 *  DESCRIPTION
 *      Read a section of the CS block.
 *
 *      This function is called when the Host requests the contents of a CS
 *      block, by writing to the OTA_READ_CS_BLOCK handle. This function is
 *      supported only if the application supports the OTA_READ_CS_BLOCK
 *      characteristic.
 *
 *  PARAMETERS
 *      offset [in]             Offset into the CS block to read from, in words.
 *                              This is the value written by the Host to the
 *                              Characteristic.
 *      length [in]             Number of octets to read.
 *      value [out]             Buffer to contain block contents
 *
 *  RETURNS
 *      sys_status_success: The read was successful and "value" contains valid
 *                          information.
 *      CSR_OTA_KEY_NOT_READ: The read was unsuccessful and "value" does not
 *                          contain valid information.
 *
 *----------------------------------------------------------------------------*/

static sys_status readCsBlock(uint16 offset, uint8 length, uint8 *value)
{
    /* Check the length is within the packet size and that the read does not
     * overflow the CS block.
     */
    if ((length > MAX_DATA_LENGTH) ||
        (offset + BYTES_TO_WORDS(length) > CSTORE_SIZE))
        return CSR_OTA_KEY_NOT_READ;

    MemCopyUnPack(value, (uint16 *)(DATA_CSTORE_START + offset), length);

    return sys_status_success;
}

/*=============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      OtaDataInit
 *
 *  DESCRIPTION
 *      This function is used to initialise OTA service data
 *      structure.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/

extern void OtaDataInit(void)
{

    ota_serv_data.ota_reset_required = FALSE;

}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      OtaHandleAccessRead
 *
 *  DESCRIPTION
 *      Handle read-access requests from the Host where the characteristic
 *      handle falls within the range of the OTAU Application Service.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void OtaHandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{
    sys_status  rc = sys_status_success;
    uint8 *p_value = NULL;
    uint8 data_length = 0;

    csr_application_id current_app;

    switch(p_ind->handle)
    {
        case HANDLE_CSR_OTA_CURRENT_APP:
        {
            /* Read the index of the current application */
            current_app = OtaReadCurrentApp();

            p_value = (uint8*)&current_app;
            data_length = 1;
        }
        break;

        case HANDLE_CSR_OTA_DATA_TRANSFER:
        {
            /* Read the value of the data transfer characteristic */
            p_value = (uint8*)data_transfer_memory;
            data_length = data_transfer_data_length;
        }
        break;

        case HANDLE_CSR_OTA_DATA_TRANSFER_CLIENT_CONFIG:
        {
            /* Read the value of the Data Transfer Client Characteristic
             * Configuration Descriptor
             */
            p_value = (uint8 *)data_transfer_configuration;
            data_length = 2;
        }
        break;

        case HANDLE_CSR_OTA_VERSION:
        {
            /* Let the firmware handle reads on this service.
             * Send response indication 
             */
            rc = gatt_status_irq_proceed;
        }
        break;

        default:
        /* Reading is not supported on this handle */
        rc = gatt_status_read_not_permitted;
        break;
    }

    GattAccessRsp(p_ind->cid, p_ind->handle, rc, data_length, p_value);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      OtaHandleAccessWrite
 *
 *  DESCRIPTION
 *      Handle write-access requests from the Host where the characteristic
 *      handle falls within the range of the OTAU Application Service.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/

extern void OtaHandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{
    sys_status rc = gatt_status_write_not_permitted;
    uint16 client_config;
    BD_ADDR_T *pBDAddr = NULL;

#if defined(USE_STATIC_RANDOM_ADDRESS)
    pBDAddr = &(DEVICE_RANDOM_ADDR);
#endif /* USE_STATIC_RANDOM_ADDRESS */

    switch(p_ind->handle)
    {
        /* When the OTAU Host writes to this characteristic, it causes the
         * device to reboot to the OTAU boot-loader or, (if supported), the
         * specified application
         */
        case HANDLE_CSR_OTA_CURRENT_APP:
        {
            /* Set the index of the current application */
            const uint8 app_id = p_ind->value[0]; /* New application index */

            /* As android does not perform service discovery on every 
             * reconnection we need to forcefully send a service changed ind
             * from the bootloader. Hence added the below hack. This can be 
             * removed once the android performs a forceful service discovery 
             * on every reconnection.
             */
            rc = OtaWriteCurrentApp(app_id,
                                  IS_PAIRED,
                                  &(CONN_CENTRAL_ADDR),
                                  0,
                                  pBDAddr,NULL,
                                  GattServiceChangedIndActive());

            if (rc != sys_status_success)
            {
                /* Sanitise the result. If OtaWriteCurrentApp fails it will be
                 * because one or more of the supplied parameters was invalid.
                 */
                rc = gatt_status_invalid_param_value;
            }
        }
        break;

        /* The OTAU Host uses this characteristic to request a specific
         * configuration key from the application. The value of the
         * configuration key is returned through a Data Transfer characteristic
         */
        case HANDLE_CSR_OTA_READ_CS_BLOCK:
        {
            /* Set the offset and length of a block of CS to read and
             * validate input(expecting uint16[2])
             */
            if (p_ind->size_value == WORDS_TO_BYTES(sizeof(uint16[2])))
            {
                const uint16 offset = BufReadUint16(&p_ind->value);

                data_transfer_data_length = (uint8)BufReadUint16(&p_ind->value);

                rc = readCsBlock(offset,
                                 data_transfer_data_length,
                                 data_transfer_memory);
            }
            else
            {
                rc = gatt_status_invalid_length;
            }
        }
        break;

        /* OTAU Host subscribes for notifications on this characteristic
         * before requesting the configuration keys from the application.
         */
        case HANDLE_CSR_OTA_DATA_TRANSFER_CLIENT_CONFIG:
        {
            /* Modify the Data Transfer Client Characteristic Configuration
             * Descriptor
             */
            client_config = BufReadUint16(&(p_ind->value));

            if((client_config == gatt_client_config_notification) ||
               (client_config == gatt_client_config_none))
            {
                data_transfer_configuration[0] = client_config;
                rc = sys_status_success;
            }
            else
            {
                /* INDICATION or RESERVED */

                /* Return error as only notifications are supported */
                rc = gatt_status_desc_improper_config;
            }
        }
        break;

        default:
        /* Writing to this characteristic is not permitted */
        break;
    }

    GattAccessRsp(p_ind->cid, p_ind->handle, rc, 0, NULL);

    /* Perform now any follow-up actions */
    if (rc == sys_status_success)
    {
        switch(p_ind->handle)
        {
            case HANDLE_CSR_OTA_READ_CS_BLOCK:
            {
                /* If this write action was to trigger a CS block read, and the
                 * notifications have been enabled, send the result now.
                 */
                if(data_transfer_configuration[0] ==
                   gatt_client_config_notification)
                {
                    GattCharValueNotification(CONNECTION_CID,
                                              HANDLE_CSR_OTA_DATA_TRANSFER,
                                              data_transfer_data_length,
                                              data_transfer_memory);
                }
            }
            break;

            case HANDLE_CSR_OTA_CURRENT_APP:
            {
                /* If a new application index has been requested disconnect from
                 * the Host and reset the device to run the new application.
                 */

                /* When the disconnect confirmation comes in, call
                 * OtaReset().
                 */
                ota_serv_data.ota_reset_required = TRUE;

                /* Disconnect from the remote device */
                AppSetState(app_state_disconnecting);

            }
            break;

            default:
                /* No follow up action necessary */
                break;
        }
    }
}

/*-----------------------------------------------------------------------------*
 *   NAME
 *       OtaCheckHandleRange
 *
 *   DESCRIPTION
 *      Determine whether a characteristic handle is within the range of the
 *      OTAU Application Service.
 *
 *   RETURNS
 *       Nothing.
 *
 *----------------------------------------------------------------------------*/

extern bool OtaCheckHandleRange(uint16 handle)
{
    return ((handle >= HANDLE_CSR_OTA_SERVICE) &&
            (handle <= HANDLE_CSR_OTA_SERVICE_END))
            ? TRUE : FALSE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      OtaResetRequired
 *
 *  DESCRIPTION
 *      This function indicates whether the OTA module requires the
 *      device to reset on client disconnection.
 *
 *  RETURNS
 *      Boolean: TRUE (OTA Reset Required) OR
 *               FALSE (OTA Reset not Required)
 *
 *---------------------------------------------------------------------------*/

extern bool OtaResetRequired(void)
{
    return ota_serv_data.ota_reset_required;
}

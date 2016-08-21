/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      csr_mesh_switch_gatt.c
 *
 *  DESCRIPTION
 *      Implementation of the CSR Mesh switch GATT-related routines
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <ls_app_if.h>
#include <gap_app_if.h>
#include <gap_types.h>
#include <ls_err.h>
#include <ls_types.h>
#include <panic.h>
#include <gatt.h>
#include <gatt_uuid.h>
#include <timer.h>
#include <debug.h>
#include <random.h>
#include <mem.h>
#include <config_store.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "app_debug.h"
#include "app_gatt.h"
#include "csr_mesh.h"
#include "appearance.h"
#include "gap_service.h"
#include "app_gatt_db.h"
#include "user_config.h"
#include "csr_mesh_switch.h"
#include "csr_mesh_switch_gatt.h"
#include "csr_mesh_switch_util.h"
#include "mesh_control_service.h"
#include "mesh_control_service_uuids.h"
#include "csr_sched.h"
#include "csr_ota_service.h"
#include "gatt_service.h"

/*============================================================================*
 *  Private Data
 *===========================================================================*/
CsrUint8 ad_data[MAX_USER_ADV_DATA_LEN];
CsrUint8 scan_rsp_data[MAX_USER_ADV_DATA_LEN];

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/
/*----------------------------------------------------------------------------*
 *  NAME
 *      getSupported16BitUUIDServiceList
 *
 *  DESCRIPTION
 *      This function prepares the list of supported 16-bit service UUIDs to be 
 *      added to Advertisement data. It also adds the relevant AD Type to the 
 *      starting of AD array.
 *
 *  RETURNS
 *      Return the size AD Service UUID data.
 *
 *---------------------------------------------------------------------------*/
static uint16 getSupported16BitUUIDServiceList(uint8 *p_service_uuid_ad)
{
    uint8 i = 0;

    /* Add 16-bit UUID for supported main service  */
    p_service_uuid_ad[i++] = AD_TYPE_SERVICE_UUID_16BIT_LIST;

    p_service_uuid_ad[i++] = LE8_L(MESH_CONTROL_SERVICE_UUID);
    p_service_uuid_ad[i++] = LE8_H(MESH_CONTROL_SERVICE_UUID);

    return ((uint16)i);

}
/*----------------------------------------------------------------------------*
 *  NAME
 *     csrStoreUserAdvData
 *
 * DESCRIPTION
 *     This function is called by the application to add either advertising or scan 
 *     response data. Each call to the function will add a single AD Structure 
 *     (refer to Bluetooth specification Vol.3 Part C Section 11). Repeated calls 
 *     will append new structures, to build up the data content.
 *     The application should not include the "length" parameter within the supplied 
 *     octet array - the GAP layer will add the length field in the appropriate 
 *     position. The first octet of the array should be the AD Type field (see ad_type) 
 *
 *  PARAMETERS
 *      param Pointer to a structure containing Advertising and Scan
 *           timing parameters to be set.
 *
 *  RETURNS/MODIFIES
 *      TRUE if data was stored successfully
 *
 *----------------------------------------------------------------------------*/
static bool csrStoreUserAdvData(uint16 len, uint8 *data, ad_src const src)
{
    uint8 offset;
    uint8 *p_data;

    /* Move to the end of the already stored data */
    if(src == ad_src_advertise)
    {
        p_data = ad_data;
    }
    else
    {
        p_data = scan_rsp_data;
    }

    if(data == NULL)
    {
        /* Data is NULL clear the data */
        MemSet(p_data, 0, MAX_USER_ADV_DATA_LEN);
        return TRUE;
    }

    offset = 0;
    while( offset < MAX_USER_ADV_DATA_LEN)
    {
        if(p_data[offset] == 0)
        {
            break;
        }
        offset += p_data[offset] + 1;
    }
    if( (offset + len) < MAX_USER_ADV_DATA_LEN )
    {
        p_data[offset] = (uint8)len;
        MemCopy(p_data + offset + 1, data, len);
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      addDeviceNameToAdvData
 *
 *  DESCRIPTION
 *      This function is used to add device name to advertisement or scan 
 *      response data. It follows below steps:
 *      a. Try to add complete device name to the advertisment packet
 *      b. Try to add complete device name to the scan response packet
 *      c. Try to add shortened device name to the advertisement packet
 *      d. Try to add shortened (max possible) device name to the scan 
 *         response packet
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void addDeviceNameToAdvData(uint16 adv_data_len, uint16 scan_data_len)
{

    uint8 *p_device_name = NULL;
    uint16 device_name_adtype_len;

    /* Read device name along with AD Type and its length */
    p_device_name = GapGetNameAndLength(&device_name_adtype_len);

    /* Add complete device name to Advertisement data */
    p_device_name[0] = AD_TYPE_LOCAL_NAME_COMPLETE;

    /* Increment device_name_length by one to account for length field
     * which will be added by the GAP layer. 
     */

    /* Check if Complete Device Name can fit in remaining advertisement 
     * data space 
     */
    if((device_name_adtype_len + 1) <= (MAX_ADV_DATA_LEN - adv_data_len))
    {
        /* Add Complete Device Name to Advertisement Data */
        if (csrStoreUserAdvData(device_name_adtype_len , p_device_name, 
                      ad_src_advertise) != TRUE)
        {
            ReportPanic(app_panic_set_advert_data);
        }

    }
    /* Check if Complete Device Name can fit in Scan response message */
    else if((device_name_adtype_len + 1) <= (MAX_ADV_DATA_LEN - scan_data_len)) 
    {
        /* Add Complete Device Name to Scan Response Data */
        if (csrStoreUserAdvData(device_name_adtype_len , p_device_name, 
                      ad_src_scan_rsp) != TRUE)
        {
            ReportPanic(app_panic_set_scan_rsp_data);
        }

    }
    /* Check if Shortened Device Name can fit in remaining advertisement 
     * data space 
     */
    else if((MAX_ADV_DATA_LEN - adv_data_len) >=
            (SHORTENED_DEV_NAME_LEN + 2)) /* Added 2 for Length and AD type 
                                           * added by GAP layer
                                           */
    {
        /* Add shortened device name to Advertisement data */
        p_device_name[0] = AD_TYPE_LOCAL_NAME_SHORT;

       if (csrStoreUserAdvData(SHORTENED_DEV_NAME_LEN , p_device_name, 
                      ad_src_advertise) != TRUE)
        {
            ReportPanic(app_panic_set_advert_data);
        }

    }
    else /* Add device name to remaining Scan reponse data space */
    {
        /* Add as much as can be stored in Scan Response data */
        p_device_name[0] = AD_TYPE_LOCAL_NAME_SHORT;

       if (csrStoreUserAdvData(MAX_ADV_DATA_LEN - scan_data_len, 
                                    p_device_name, 
                                    ad_src_scan_rsp) != TRUE)
        {
            ReportPanic(app_panic_set_scan_rsp_data);
        }

    }

}
/*----------------------------------------------------------------------------*
 *  NAME
 *      gattSetAdvertData
 *
 *  DESCRIPTION
 *      This function is used to set advertisement data 
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void gattSetAdvertData(void)
{
    uint8 advert_data[MAX_ADV_DATA_LEN];
    uint16 length;

    int8 tx_power_level = 0xff; /* Signed value */

    /* Tx power level value prefixed with 'Tx Power' AD Type */
    uint8 device_tx_power[TX_POWER_VALUE_LENGTH] = {
                AD_TYPE_TX_POWER
                };

    uint8 device_appearance[ATTR_LEN_DEVICE_APPEARANCE + 1] = {
                AD_TYPE_APPEARANCE,
                LE8_L(APPEARANCE_UNKNOWN_VALUE),
                LE8_H(APPEARANCE_UNKNOWN_VALUE)
                };

    /* A variable to keep track of the data added to AdvData. The limit is 
     * MAX_ADV_DATA_LEN. GAP layer will add AD Flags to AdvData which 
     * is 3 bytes. Refer BT Spec 4.0, Vol 3, Part C, Sec 11.1.3.
     */
    uint16 length_added_to_adv = 3;

    /* Add UUID list of the services supported by the device */
    length = getSupported16BitUUIDServiceList(advert_data);

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (length + 1);

    if (csrStoreUserAdvData(length, advert_data, 
                                       ad_src_advertise) != TRUE)
    {
        ReportPanic(app_panic_set_advert_data);
    }

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (sizeof(device_appearance) + 1);

    /* Add device appearance to the advertisements */
    if (csrStoreUserAdvData(ATTR_LEN_DEVICE_APPEARANCE + 1, 
        device_appearance, ad_src_advertise) != TRUE)
    {
        ReportPanic(app_panic_set_advert_data);
    }

    /* Read tx power of the chip */
    if(LsReadTransmitPowerLevel(&tx_power_level) != ls_err_none)
    {
        /* Reading tx power failed */
        ReportPanic(app_panic_read_tx_pwr_level);
    }

    /* Add the read tx power level to device_tx_power 
      * Tx power level value is of 1 byte 
      */
    device_tx_power[TX_POWER_VALUE_LENGTH - 1] = (uint8 )tx_power_level;

    /* One added for Length field, which will be added to Adv Data by GAP 
     * layer 
     */
    length_added_to_adv += (TX_POWER_VALUE_LENGTH + 1);

    /* Add tx power value of device to the advertising data */
    if (csrStoreUserAdvData(TX_POWER_VALUE_LENGTH, device_tx_power, 
                          ad_src_advertise) != TRUE)
    {
        ReportPanic(app_panic_set_advert_data);
    }

    addDeviceNameToAdvData(length_added_to_adv, 0);

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      gattAdvertTimerHandler
 *
 *  DESCRIPTION
 *      This function is used to handle Gatt advertisement timer expiry.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
static void gattAdvertTimerHandler(timer_id tid)
{
    if (g_switchapp_data.gatt_data.gatt_advert_tid == tid)
    {
        g_switchapp_data.gatt_data.gatt_advert_tid = TIMER_INVALID;

            /* After the timer expiry switch to fast advertising if device is
             * not connected.
             */
        GattTriggerConnectableAdverts();
    }
}

/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAccessWrite
 *
 *  DESCRIPTION
 *      This function handles Write operation on attributes (as received in 
 *      GATT_ACCESS_IND message) maintained by the application.
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/
extern void HandleAccessWrite(GATT_ACCESS_IND_T *p_ind)
{

    /* For the received attribute handle, check all the services that support 
     * attribute 'Write' operation handled by application.
     */

    if(GapCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to GAP service */
        GapHandleAccessWrite(p_ind);
    }
    else if(MeshControlCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to Mesh Control service */
        MeshControlHandleAccessWrite(p_ind);
    }
    else if(GattCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to Gatt service */
        GattHandleAccessWrite(p_ind);
    }
    else if(OtaCheckHandleRange(p_ind->handle))
    {
        OtaHandleAccessWrite(p_ind);
    }
    else
    {
        /* Application doesn't support 'Write' operation on received 
         * attribute handle, hence return 'gatt_status_write_not_permitted'
         * status
         */
        GattAccessRsp(p_ind->cid, p_ind->handle, 
                      gatt_status_write_not_permitted,
                      0, NULL);
    }

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      HandleAccessRead
 *
 *  DESCRIPTION
 *      This function handles read operation on attributes (as received in 
 *      GATT_ACCESS_IND message) maintained by the application and respond 
 *      with the GATT_ACCESS_RSP message.
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/
extern void HandleAccessRead(GATT_ACCESS_IND_T *p_ind)
{

    /* For the received attribute handle, check all the services that support 
     * attribute 'Read' operation handled by application.
     */

    if(GapCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to GAP service */
        GapHandleAccessRead(p_ind);
    }
    else if(MeshControlCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to Mesh Control service */
        MeshControlHandleAccessRead(p_ind);
    }
    else if(GattCheckHandleRange(p_ind->handle))
    {
        /* Attribute handle belongs to Gatt service */
        GattHandleAccessRead(p_ind);
    }
    else if(OtaCheckHandleRange(p_ind->handle))
    {
        OtaHandleAccessRead(p_ind);
    }
    else
    {
        /* Application doesn't support 'Read' operation on received 
         * attribute handle, hence return 'gatt_status_read_not_permitted'
         * status
         */
        GattAccessRsp(p_ind->cid, p_ind->handle, 
                      gatt_status_read_not_permitted,
                      0, NULL);
    }

}

/*------------------------------------------------------------*
 *  NAME
 *      GattIsAddressResolvableRandom
 *
 *  DESCRIPTION
 *      This function checks if the address is resolvable random or not.
 *
 *  RETURNS
 *      Boolean - True (Resolvable Random Address) /
 *                     False (Not a Resolvable Random Address)
 *
 *----------------------------------------------------------*/
extern bool GattIsAddressResolvableRandom(TYPED_BD_ADDR_T *p_addr)
{
    if(p_addr->type != L2CA_RANDOM_ADDR_TYPE || 
       (p_addr->addr.nap & BD_ADDR_NAP_RANDOM_TYPE_MASK)
                                      != BD_ADDR_NAP_RANDOM_TYPE_RESOLVABLE)
    {
        /* This isn't a resolvable private address */
        return FALSE;
    }
    return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattTriggerConnectableAdverts
 *
 *  DESCRIPTION
 *      This function is used to trigger fast advertisements 
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/
extern void GattTriggerConnectableAdverts(void)
{
    CSRSchedResult result = CSR_SCHED_RESULT_FAILURE;
    CSR_SCHED_ADV_DATA_T gatt_adv_data;

    /* Bridge is disabled do not send user adverts */
    if(!(g_switchapp_data.bearer_tx_state.bearerEnabled &
                                                     GATT_SERVER_BEARER_ACTIVE))
    {
        AppSetState(app_state_idle);
        return;
    }

    /* Reset existing advertising data */
    csrStoreUserAdvData(0, NULL, ad_src_advertise);
    csrStoreUserAdvData(0, NULL, ad_src_scan_rsp);

#ifdef USE_STATIC_RANDOM_ADDRESS
    /* Restore the Random Address of the Bluetooth Device */
    MemCopy(&gatt_adv_data.adv_params.bd_addr.addr, 
            &g_switchapp_data.random_bd_addr, 
            sizeof(BD_ADDR_T));
    gatt_adv_data.adv_params.bd_addr.type = L2CA_RANDOM_ADDR_TYPE;
#else
    gatt_adv_data.adv_params.bd_addr.type = L2CA_PUBLIC_ADDR_TYPE;
#endif /* USE_STATIC_RANDOM_ADDRESS */


    /* Set GAP peripheral params */
    gatt_adv_data.adv_params.role = gap_role_peripheral;
    gatt_adv_data.adv_params.bond = gap_mode_bond_no;
    gatt_adv_data.adv_params.connect_mode = gap_mode_connect_undirected;
    gatt_adv_data.adv_params.discover_mode = gap_mode_discover_general;
    gatt_adv_data.adv_params.security_mode = gap_mode_security_unauthenticate;

    /* Form the ad type data */
    gattSetAdvertData();

    MemCopy(gatt_adv_data.ad_data, ad_data, MAX_USER_ADV_DATA_LEN);
    gatt_adv_data.ad_data_length = MAX_USER_ADV_DATA_LEN;
    MemCopy(gatt_adv_data.scan_rsp_data, scan_rsp_data, MAX_USER_ADV_DATA_LEN);
    gatt_adv_data.scan_rsp_data_length= MAX_USER_ADV_DATA_LEN;

    result = CSRSchedSendUserAdv(&gatt_adv_data, NULL);

    g_switchapp_data.gatt_data.advert_timer_value = ADVERT_INTERVAL;

    /* Restart the advertising timer */
    TimerDelete(g_switchapp_data.gatt_data.gatt_advert_tid);
    g_switchapp_data.gatt_data.gatt_advert_tid = TimerCreate(
                               g_switchapp_data.gatt_data.advert_timer_value,
                                                TRUE, gattAdvertTimerHandler);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      GattStopAdverts
 *
 *  DESCRIPTION
 *      This function is used to stop on-going advertisements.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void GattStopAdverts(void)
{
    TimerDelete(g_switchapp_data.gatt_data.gatt_advert_tid);
    g_switchapp_data.gatt_data.gatt_advert_tid = TIMER_INVALID;
}


/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      csr_mesh_bridge_util.c
 *
 *
 ******************************************************************************/
 /*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <main.h>
#include <ls_app_if.h>
#include <gatt.h>
#include <timer.h>
#include <uart.h>
#include <pio.h>
#include <nvm.h>
#include <security.h>
#include <gatt_prim.h>
#include <mem.h>
#include <panic.h>
#include <config_store.h>
#include <random.h>
#include <buf_utils.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"
#include "nvm_access.h"
#include "app_debug.h"
#include "app_gatt.h"
#include "gap_service.h"
#include "app_gatt_db.h"
#include "csr_mesh_model_common.h"
#include "csr_mesh_bridge.h"
#include "csr_mesh_bridge_util.h"
#include "csr_mesh_bridge_gatt.h"
#include "mesh_control_service.h"
#include "mesh_control_service_uuids.h"
#include "appearance.h"

/*============================================================================*
 *  Private Data
 *============================================================================*/
/* CSRmesh Advert Data */
uint8 mesh_ad_data[3] = {(AD_TYPE_SERVICE_DATA_UUID_16BIT),
                                (MTL_ID_CODE & 0x00FF),
                                ((MTL_ID_CODE & 0xFF00 ) >> 8)};

/* Device Apprearance. */
CSR_MESH_DEVICE_APPEARANCE_T appearance = {"bridge2.0",
                                           {APPEARANCE_ORG_BLUETOOTH_SIG,
                                            APPEARANCE_CSRMESH_BRIDGE_VALUE,
                                            0x00000000}};

/*============================================================================*
 *  Public Function Implemtations
 *============================================================================*/
/*----------------------------------------------------------------------------*
 *  NAME
 *      ReadPersistentStore
 *
 *  DESCRIPTION
 *      This function is used to initialize and read NVM data
 *
 *  RETURNS/MODIFIES
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
extern void ReadPersistentStore(void)
{
    /* NVM offset for supported services */
    uint16 nvm_offset = 0;
    uint16 nvm_sanity = 0xffff;
    uint16 app_nvm_version = 0;

    nvm_offset = NVM_MAX_APP_MEMORY_WORDS;

    /* Read the sanity word */
    Nvm_Read(&nvm_sanity, sizeof(nvm_sanity),
             NVM_OFFSET_SANITY_WORD);

    /* Read the Application NVM version */
    Nvm_Read(&app_nvm_version, 1, NVM_OFFSET_APP_NVM_VERSION);

    if(nvm_sanity == NVM_SANITY_MAGIC &&
       app_nvm_version == APP_NVM_VERSION )
    {
        /* If NVM in use, read device name and length from NVM */
        GapReadDataFromNVM(&nvm_offset);
    }
    else
    {
        /* Either the NVM Sanity is not valid or the App Version has changed */
        if( nvm_sanity != NVM_SANITY_MAGIC)
        {
            /* NVM Sanity check failed means either the device is being brought
             * up for the first time or memory has got corrupted in which case
             * discard the data and start fresh.
             */
            nvm_sanity = NVM_SANITY_MAGIC;

            /* Write device name and length to NVM for the first time */
            GapInitWriteDataToNVM(&nvm_offset);

            /* Store new version of the NVM */
            app_nvm_version = APP_NVM_VERSION;
            Nvm_Write(&app_nvm_version, 1, NVM_OFFSET_APP_NVM_VERSION);

            /* Write NVM Sanity word to NVM. Make sure to write the sanity word
             * at the end after all other NVM data is written.
             * This helps in avoiding unexpected application behaviour in case 
             * of a device reset after sanity word is written but other NVM info
             * is not written
             */
            Nvm_Write(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);
        }
    }
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppUpdateBearerState
 *
 *  DESCRIPTION
 *      This function updates the relay and promiscuous mode of the GATT and
 *      and the LE Advert bearers
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
extern void AppUpdateBearerState(CSR_MESH_TRANSMIT_STATE_T *p_bearer_state)
{
    CSR_MESH_APP_EVENT_DATA_T ret_evt_data;
    CSR_MESH_TRANSMIT_STATE_T ret_bearer_state;

    ret_evt_data.appCallbackDataPtr = &ret_bearer_state;
    CSRmeshSetTransmitState(p_bearer_state, &ret_evt_data);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      ReportPanic
 *
 *  DESCRIPTION
 *      This function calls firmware panic routine and gives a single point
 *      of debugging any application level panics
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void ReportPanic(app_panic_code panic_code)
{
    /* Raise panic */
    Panic(panic_code);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      AppPowerOnReset
 *
 *  DESCRIPTION
 *      This user application function is called just after a power-on reset
 *      (including after a firmware panic), or after a wakeup from Hibernate or
 *      Dormant sleep states.
 *
 *      At the time this function is called, the last sleep state is not yet
 *      known.
 *
 *      NOTE: this function should only contain code to be executed after a
 *      power-on reset or panic. Code that should also be executed after an
 *      HCI_RESET should instead be placed in the AppInit() function.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void AppPowerOnReset(void)
{

}

#ifdef USE_STATIC_RANDOM_ADDRESS
/*-----------------------------------------------------------------------------*
 *  NAME
 *      generateStaticRandomAddress
 *
 *  DESCRIPTION
 *      This function generates a static random address.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
extern  void generateStaticRandomAddress(BD_ADDR_T *addr)
{
    uint16 temp[3];
    uint16 idx = 0;

    if (!addr) return;

    for (idx = 0; idx < 3;)
    {
        temp[idx] = Random16();
        if ((temp[idx] != 0) && (temp[idx] != 0xFFFF))
        {
            idx++;
        }
    }

    addr->lap = ((uint32)(temp[1]) << 16) | (temp[0]);
    addr->lap &= 0x00FFFFFFUL;
    addr->uap = (temp[1] >> 8) & 0xFF;
    addr->nap = temp[2];

    addr->nap &= ~BD_ADDR_NAP_RANDOM_TYPE_MASK;
    addr->nap |=  BD_ADDR_NAP_RANDOM_TYPE_STATIC;
}
#endif /* USE_STATIC_RANDOM_ADDRESS */


/*============================================================================*
 *  Public Function Definitions
 *============================================================================*/
#ifdef NVM_TYPE_FLASH
/*----------------------------------------------------------------------------*
 *  NAME
 *      WriteApplicationAndServiceDataToNVM
 *
 *  DESCRIPTION
 *      This function writes the application data to NVM. This function should
 *      be called on getting nvm_status_needs_erase
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void WriteApplicationAndServiceDataToNVM(void)
{
    uint16 nvm_sanity = NVM_SANITY_MAGIC;
    uint16 app_nvm_version = APP_NVM_VERSION;
    
    /* Write NVM sanity word to the NVM */
    Nvm_Write(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);

    /* Writw APP NVM version to the NVM */
    Nvm_Write(&app_nvm_version, sizeof(app_nvm_version),
                                                NVM_OFFSET_APP_NVM_VERSION);

    /* Write GAP service data into NVM */
    WriteGapServiceDataInNVM();
}
#endif /* NVM_TYPE_FLASH */


/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      csr_mesh_light_util.c
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
#include "csr_mesh_light.h"
#include "csr_mesh_light_hw.h"
#include "csr_mesh_light_util.h"
#include "csr_mesh_light_gatt.h"
#include "mesh_control_service.h"
#include "mesh_control_service_uuids.h"
#include "appearance.h"
#include "iot_hw.h"

/*============================================================================*
 *  Private Data
 *============================================================================*/
/* CSRmesh Advert Data */
uint8 mesh_ad_data[3] = {(AD_TYPE_SERVICE_DATA_UUID_16BIT),
                                (MTL_ID_CODE & 0x00FF),
                                ((MTL_ID_CODE & 0xFF00 ) >> 8)};

/* Device Apprearance. */
CSR_MESH_DEVICE_APPEARANCE_T appearance = {"Light2.0",
                                           {APPEARANCE_ORG_BLUETOOTH_SIG,
                                           APPEARANCE_CSRMESH_LIGHT_VALUE,
                                           0x00000000}};

/*============================================================================*
 *  Private Function Implementations
 *============================================================================*/
 #ifdef ENABLE_DEVICE_UUID_ADVERTS
/*-----------------------------------------------------------------------------*
 *  NAME
 *      deviceIdAdvertTimeoutHandler
 *
 *  DESCRIPTION
 *      This function handles the Device ID advertise timer event.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
static void deviceIdAdvertTimeoutHandler(timer_id tid)
{

    if(tid == g_lightapp_data.mesh_device_id_advert_tid)
    {
        if(g_lightapp_data.assoc_state == app_state_not_associated)
        {
            /* Generate a random delay between 0 to 511 ms */
            uint32 random_delay = ((uint32)(Random16() & 0x1FF))*(MILLISECOND);

            DEBUG_STR("CSRMeshAssociateToANetwork  API is called---> \r\n");
            CSRmeshAssociateToANetwork(&appearance , 10);

            g_lightapp_data.mesh_device_id_advert_tid = TimerCreate(
                                         (DEVICE_ID_ADVERT_TIME + random_delay),
                                         TRUE,
                                         deviceIdAdvertTimeoutHandler);
        }
        else
        {
            /* Device is now associated so no need to start the timer again */
            TimerDelete(g_lightapp_data.mesh_device_id_advert_tid);
            g_lightapp_data.mesh_device_id_advert_tid = TIMER_INVALID;
        }
    }
}
#endif /* ENABLE_DEVICE_UUID_ADVERTS */

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
void ReadPersistentStore(void)
{
    /* NVM offset for supported services */
    uint16 nvm_offset = 0;
    uint16 nvm_sanity = 0xffff;
    uint16 app_nvm_version = 0;
    uint32 temp = 0;
    uint16 index;
    uint16 cskey_flags = CSReadUserKey(CSKEY_INDEX_USER_FLAGS);

    nvm_offset = NVM_MAX_APP_MEMORY_WORDS;

    /* Read the sanity word */
    Nvm_Read(&nvm_sanity, sizeof(nvm_sanity),
             NVM_OFFSET_SANITY_WORD);

    /* Read the Application NVM version */
    Nvm_Read(&app_nvm_version, 1, NVM_OFFSET_APP_NVM_VERSION);

    /* Initialise the paired flag to false */
    g_lightapp_data.gatt_data.paired = FALSE;

    if(nvm_sanity == NVM_SANITY_MAGIC &&
       app_nvm_version == APP_NVM_VERSION )
    {
        /* Read RGB and Power Data from NVM */
        Nvm_Read((uint16 *)&temp, sizeof(uint32),
                 NVM_RGB_DATA_OFFSET);

        /* Unpack data in to the global variables */
        g_lightapp_data.light_model.red   = temp & 0xFF;
        temp >>= 8;
        g_lightapp_data.light_model.green = temp & 0xFF;
        temp >>= 8;
        g_lightapp_data.light_model.blue  = temp & 0xFF;
        temp >>= 8;
        g_lightapp_data.power_model.state = temp & 0xFF;
        /* Keep the level at highest as default */
        g_lightapp_data.light_model.level = 0xFF;
        g_lightapp_data.light_model.power = g_lightapp_data.power_model.state;

        /* Read the saved bearer state from NVM */
        Nvm_Read((uint16 *)&g_lightapp_data.bearer_tx_state, 
                  sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                  NVM_OFFSET_BEARER_STATE);

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

            /* The device will not be associated as it is coming up for the
             * first time
             */
            g_lightapp_data.assoc_state = app_state_not_associated;

            /* Write association state to NVM */
            Nvm_Write((uint16 *)&g_lightapp_data.assoc_state,
                      sizeof(g_lightapp_data.assoc_state),
                      NVM_OFFSET_ASSOCIATION_STATE);

            if (cskey_flags & CSKEY_RANDOM_UUID_ENABLE_BIT)
            {
                uint16 uuid[UUID_LENGTH_WORDS];

                /* The flag is set so generate a random UUID and write to NVM */
                for (index = 0; index < UUID_LENGTH_WORDS; index++)
                {
                    uuid[index] = Random16();
                }
                Nvm_Write(uuid, UUID_LENGTH_WORDS,
                          CSR_MESH_NVM_DEVICE_UUID_OFFSET);

#ifdef USE_AUTHORISATION_CODE
                uint16 ac[AUTH_CODE_LENGTH_WORDS];

                /* The flag is set so generate a random UUID and write to NVM */
                for (index = 0; index < AUTH_CODE_LENGTH_WORDS; index++)
                {
                    ac[index] = Random16();
                }
                Nvm_Write(ac, AUTH_CODE_LENGTH_WORDS,
                          CSR_MESH_NVM_DEVICE_AUTH_CODE_OFFSET);
#endif /* USE_AUTHORISATION_CODE */
            }

            /* Write NVM Sanity word to NVM. Make sure to write the sanity word
             * at the end after all other NVM data is written.
             * This helps in avoiding unexpected application behaviour in case 
             * of a device reset after sanity word is written but other NVM info
             * is not written
             */
            Nvm_Write(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);
        }

        /* Initialize the new version of the NVM */
        app_nvm_version = APP_NVM_VERSION;

        /* All the persistent data below will be reset to default upon an
         * application update. If some of the data needs to be retained even
         * after an application update, it has to be moved within the sanity
         * word check
         */

        /* Write RGB Data and Power to NVM.
         * Data is stored in the following format.
         * HIGH WORD: MSB: POWER LSB: BLUE.
         * LOW  WORD: MSB: GREEN LSB: RED.
         */
        g_lightapp_data.light_model.red   = 0xFF;
        g_lightapp_data.light_model.green = 0xFF;
        g_lightapp_data.light_model.blue  = 0xFF;
        g_lightapp_data.light_model.level = 0xFF;
        g_lightapp_data.light_model.power = csr_mesh_power_state_off;
        g_lightapp_data.power_model.state = csr_mesh_power_state_off;

        temp = ((uint32) g_lightapp_data.power_model.state << 24) |
               ((uint32) g_lightapp_data.light_model.blue  << 16) |
               ((uint32) g_lightapp_data.light_model.green <<  8) |
               g_lightapp_data.light_model.red;

        Nvm_Write((uint16 *)&temp, sizeof(uint32),
                 NVM_RGB_DATA_OFFSET);

        /* Enable relay and bridge based on the CS User Key setting */
        g_lightapp_data.bearer_tx_state.bearerEnabled =  LE_BEARER_ACTIVE;
        if( cskey_flags & CSKEY_RELAY_ENABLE_BIT)
        {
            g_lightapp_data.bearer_tx_state.bearerRelayActive = 
                               LE_BEARER_ACTIVE | GATT_SERVER_BEARER_ACTIVE;
        }
        if( cskey_flags & CSKEY_BRIDGE_ENABLE_BIT)
        {
            g_lightapp_data.bearer_tx_state.bearerEnabled  |=
                                                  GATT_SERVER_BEARER_ACTIVE;
        }
        /* Enable promiscuous mode as the device is not associated. This
         * allows the device to relay MCP messages targeted for other
         * devices that are already associated
         */
        g_lightapp_data.bearer_tx_state.bearerPromiscuous = 
                                   LE_BEARER_ACTIVE | GATT_SERVER_BEARER_ACTIVE;
        /* Save the state to NVM */
        Nvm_Write((uint16 *)&g_lightapp_data.bearer_tx_state, 
                  sizeof(CSR_MESH_BEARER_STATE_DATA_T),
                  NVM_OFFSET_BEARER_STATE);

        /* Initialize model groups */
        MemSet(light_model_groups, 0x0000, sizeof(uint16)*MAX_MODEL_GROUPS);
        Nvm_Write((uint16 *)light_model_groups, sizeof(uint16)*MAX_MODEL_GROUPS,
                                                 NVM_OFFSET_LIGHT_MODEL_GROUPS);

        MemSet(power_model_groups, 0x0000, sizeof(uint16)*MAX_MODEL_GROUPS);
        Nvm_Write((uint16 *)power_model_groups, sizeof(uint16)*MAX_MODEL_GROUPS,
                                                 NVM_OFFSET_POWER_MODEL_GROUPS);

        MemSet(attention_model_groups, 0x0000, sizeof(uint16)*MAX_MODEL_GROUPS);
        Nvm_Write((uint16 *)attention_model_groups, 
            sizeof(uint16)*MAX_MODEL_GROUPS, NVM_OFFSET_ATT_MODEL_GROUPS);

#ifdef ENABLE_DATA_MODEL
        MemSet(data_model_groups, 0x0000, sizeof(uint16)*MAX_MODEL_GROUPS);
        Nvm_Write((uint16 *)data_model_groups, 
            sizeof(uint16)*MAX_MODEL_GROUPS, NVM_OFFSET_DATA_MODEL_GROUPS);
#endif /* ENABLE_DATA_MODEL */

        /* Write device name and length to NVM for the first time */
        GapInitWriteDataToNVM(&nvm_offset);

        /* Write app NVM version to NVM. Make sure to write the app nvm version
         * at the end after all other NVM data is written.
         * This helps in avoiding unexpected application behaviour in case 
         * of a device reset after sanity word is written but other NVM info
         * is not written
         */
        Nvm_Write(&app_nvm_version, 1, NVM_OFFSET_APP_NVM_VERSION);
    }
    
    /* Read assigned Groups IDs for Light model from NVM */
    Nvm_Read((uint16 *)light_model_groups, sizeof(uint16)*MAX_MODEL_GROUPS,
                                             NVM_OFFSET_LIGHT_MODEL_GROUPS);

    /* Read assigned Groups IDs for Power model from NVM */
    Nvm_Read((uint16 *)power_model_groups, sizeof(uint16)*MAX_MODEL_GROUPS,
                                             NVM_OFFSET_POWER_MODEL_GROUPS);

    /* Read assigned Groups IDs for Attention model from NVM */
    Nvm_Read((uint16 *)attention_model_groups, sizeof(uint16)*MAX_MODEL_GROUPS,
                                             NVM_OFFSET_ATT_MODEL_GROUPS);

#ifdef ENABLE_DATA_MODEL
    /* Read assigned Groups IDs for Data model from NVM */
    Nvm_Read((uint16 *)data_model_groups, sizeof(uint16)*MAX_MODEL_GROUPS,
                                             NVM_OFFSET_DATA_MODEL_GROUPS);
#endif /* ENABLE_DATA_MODEL */

    /* Read association state from NVM */
    Nvm_Read((uint16 *)&g_lightapp_data.assoc_state,
            sizeof(g_lightapp_data.assoc_state), NVM_OFFSET_ASSOCIATION_STATE);
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      InitiateAssociation
 *
 *  DESCRIPTION
 *      This function kick starts a timer to send periodic CSRmesh UUID Messages
 *      and starts blinking Blue LED to visually indicate association ready 
 *      status
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
extern void InitiateAssociation(void)
{
    /* Blink light to indicate that it is not associated */
    IOTLightControlDeviceBlink(0, 0, 127, 32, 32);

#ifdef ENABLE_DEVICE_UUID_ADVERTS
    CSRmeshAssociateToANetwork(&appearance , 10);

    /* Start a timer to send Device ID messages periodically to get
     * associated to a network
     */
    g_lightapp_data.mesh_device_id_advert_tid =
                                    TimerCreate(DEVICE_ID_ADVERT_TIME,
                                                TRUE,
                                                deviceIdAdvertTimeoutHandler);
#endif /* ENABLE_DEVICE_UUID_ADVERTS */
}

/*-----------------------------------------------------------------------------*
 *  NAME
 *      RemoveAssociation
 *
 *  DESCRIPTION
 *      This function removes the associtation and moves back the device to 
 *      start associating again.
 *
 *  RETURNS/MODIFIES
 *      Nothing
 *
 *----------------------------------------------------------------------------*/
extern void RemoveAssociation(void)
{
    uint32 wr_data = 0;

    /* Enable promiscuous mode */
    g_lightapp_data.bearer_tx_state.bearerPromiscuous = 
                LE_BEARER_ACTIVE | GATT_SERVER_BEARER_ACTIVE;
    AppUpdateBearerState(&g_lightapp_data.bearer_tx_state);

    /* Save the state to NVM */
    Nvm_Write((uint16 *)&g_lightapp_data.bearer_tx_state, 
              sizeof(CSR_MESH_BEARER_STATE_DATA_T),
              NVM_OFFSET_BEARER_STATE);

    g_lightapp_data.assoc_state = app_state_not_associated;

    /* Write association state to NVM */
    Nvm_Write((uint16 *)&g_lightapp_data.assoc_state,
             sizeof(g_lightapp_data.assoc_state),
             NVM_OFFSET_ASSOCIATION_STATE);

    /* Reset the supported model groups and save it to NVM */
    /* Light model */
    MemSet(light_model_groups, 0x0000, 
                                sizeof(light_model_groups));
    Nvm_Write((uint16 *)light_model_groups, 
                                sizeof(light_model_groups),
                                NVM_OFFSET_LIGHT_MODEL_GROUPS);

    /* Power model */
    MemSet(power_model_groups, 0x0000, 
                                sizeof(power_model_groups));
    Nvm_Write((uint16 *)power_model_groups, 
                                sizeof(power_model_groups),
                                NVM_OFFSET_POWER_MODEL_GROUPS);

    /* attention model */
    MemSet(attention_model_groups, 0x0000, 
                                sizeof(attention_model_groups));
    Nvm_Write((uint16 *)attention_model_groups, 
                                sizeof(attention_model_groups),
                                NVM_OFFSET_ATT_MODEL_GROUPS);

#ifdef ENABLE_DATA_MODEL
    /* data model */
    MemSet(data_model_groups, 0x0000, 
                                sizeof(data_model_groups));
    Nvm_Write((uint16 *)data_model_groups, 
                                sizeof(data_model_groups),
                                NVM_OFFSET_DATA_MODEL_GROUPS);
#endif /* ENABLE_DATA_MODEL */

    /* Reset Light State */
    g_lightapp_data.light_model.red   = 0xFF;
    g_lightapp_data.light_model.green = 0xFF;
    g_lightapp_data.light_model.blue  = 0xFF;
    g_lightapp_data.light_model.level = 0xFF;
    g_lightapp_data.light_model.power =csr_mesh_power_state_off;
    g_lightapp_data.power_model.state =csr_mesh_power_state_off;

    /* Pack Data for writing to NVM */
    wr_data = 
        ((uint32) g_lightapp_data.power_model.state << 24) |
        ((uint32) g_lightapp_data.light_model.blue  << 16) |
        ((uint32) g_lightapp_data.light_model.green <<  8) |
        g_lightapp_data.light_model.red;

   Nvm_Write((uint16 *)&wr_data, 
                        sizeof(uint32),NVM_RGB_DATA_OFFSET);

    /* Association is removed from configuring device. Initiate association */
    InitiateAssociation();
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
 *      HandleGroupSetMsg
 *
 *  DESCRIPTION
 *      This function handles the CSRmesh Group Assignment message. Stores
 *      the group_id at the given index for the model
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern bool HandleGroupSetMsg(CSR_MESH_GROUP_ID_RELATED_DATA_T msg)
{
    CSRMESH_MODEL_TYPE_T model = msg.model;
    uint8 index = msg.gpIdx;
    uint16 group_id = msg.gpId;
    bool update_lastetag = TRUE;

    if(model == CSRMESH_LIGHT_MODEL || model == CSRMESH_ALL_MODELS)
    {
        if(index < MAX_MODEL_GROUPS)
        {
            /* Store Group ID */
            light_model_groups[index] = group_id;

            /* Save to NVM */
            Nvm_Write(&light_model_groups[index],
                      sizeof(uint16),
                      NVM_OFFSET_LIGHT_MODEL_GROUPS + index);
        }
        else
        {
            update_lastetag = FALSE;
        }
    }

    if(model == CSRMESH_POWER_MODEL || model == CSRMESH_ALL_MODELS)
    {
        if(index < MAX_MODEL_GROUPS)
        {
            power_model_groups[index] = group_id;

            /* Save to NVM */
            Nvm_Write(&power_model_groups[index],
                      sizeof(uint16),
                      NVM_OFFSET_POWER_MODEL_GROUPS + index);
        }
        else
        {
            update_lastetag = FALSE;
        }
    }

    if(model == CSRMESH_ATTENTION_MODEL || model == CSRMESH_ALL_MODELS)
    {
        if(index < MAX_MODEL_GROUPS)
        {
            attention_model_groups[index] = group_id;

            /* Save to NVM */
            Nvm_Write(&attention_model_groups[index],
                      sizeof(uint16),
                      NVM_OFFSET_ATT_MODEL_GROUPS + index);
        }
        else
        {
            update_lastetag = FALSE;
        }
    }

#ifdef ENABLE_DATA_MODEL
    if(model == CSRMESH_DATA_MODEL || model == CSRMESH_ALL_MODELS)
    {
        if(index < MAX_MODEL_GROUPS)
        {
            data_model_groups[index] = group_id;

            /* Save to NVM */
            Nvm_Write(&data_model_groups[index],
                      sizeof(uint16),
                      NVM_OFFSET_DATA_MODEL_GROUPS + index);
        }
        else
        {
            update_lastetag = FALSE;
        }
    }
#endif /* ENABLE_DATA_MODEL */

    return update_lastetag;
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
extern  void GenerateStaticRandomAddress(BD_ADDR_T *addr)
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
    uint16 nvm_sanity = 0xffff;
    nvm_sanity = NVM_SANITY_MAGIC;
    uint32 wr_data = 0;
    uint16 index = 0;

    /* Write NVM sanity word to the NVM */
    Nvm_Write(&nvm_sanity, sizeof(nvm_sanity), NVM_OFFSET_SANITY_WORD);

    /* Store the Association State */
    Nvm_Write((uint16 *)&g_lightapp_data.assoc_state,
             sizeof(g_lightapp_data.assoc_state),
              NVM_OFFSET_ASSOCIATION_STATE);

    /* Pack Data for writing to NVM */
    wr_data = 
        ((uint32) g_lightapp_data.power_model.state << 24) |
        ((uint32) g_lightapp_data.light_model.blue  << 16) |
        ((uint32) g_lightapp_data.light_model.green <<  8) |
        g_lightapp_data.light_model.red;

   Nvm_Write((uint16 *)&wr_data, 
                        sizeof(uint32),NVM_RGB_DATA_OFFSET);

    for(index = 0; index < MAX_MODEL_GROUPS; index++)
    {
        /* Save to NVM */
        Nvm_Write(&light_model_groups[index],
                  sizeof(uint16),
                  NVM_OFFSET_LIGHT_MODEL_GROUPS + index);

        Nvm_Write(&power_model_groups[index],
                  sizeof(uint16),
                  NVM_OFFSET_POWER_MODEL_GROUPS + index);

        Nvm_Write(&attention_model_groups[index],
                  sizeof(uint16),
                  NVM_OFFSET_ATT_MODEL_GROUPS + index);

#ifdef ENABLE_DATA_MODEL
        Nvm_Write(&data_model_groups[index],
                  sizeof(uint16),
                  NVM_OFFSET_DATA_MODEL_GROUPS + index);
#endif /* ENABLE_DATA_MODEL */
    }

    /* Write GAP service data into NVM */
    WriteGapServiceDataInNVM();
}
#endif /* NVM_TYPE_FLASH */



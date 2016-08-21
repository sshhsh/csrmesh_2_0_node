/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      app_fw_event_handler.h
 *
 *
 ******************************************************************************/
#ifndef __APP_FW_EVENT_HANDLER_H__
#define __APP_FW_EVENT_HANDLER_H__

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

extern void HandleSignalGattAddDBCfm(GATT_ADD_DB_CFM_T *p_event_data);
extern void HandleSignalGattCancelConnectCfm(
                                    GATT_CANCEL_CONNECT_CFM_T *p_event_data);
extern void HandleSignalLmEvConnectionComplete(
                                     LM_EV_CONNECTION_COMPLETE_T *p_event_data);
extern void HandleSignalGattConnectCfm(GATT_CONNECT_CFM_T* p_event_data);
extern void HandleSignalSmSimplePairingCompleteInd(
                                SM_SIMPLE_PAIRING_COMPLETE_IND_T *p_event_data);
extern void HandleSignalLsConnParamUpdateCfm(
                            LS_CONNECTION_PARAM_UPDATE_CFM_T *p_event_data);
extern void HandleSignalLmConnectionUpdate(
                                   LM_EV_CONNECTION_UPDATE_T* p_event_data);
extern void HandleSignalLsConnParamUpdateInd(
                                LS_CONNECTION_PARAM_UPDATE_IND_T *p_event_data);
extern void HandleSignalGattAccessInd(GATT_ACCESS_IND_T *p_event_data);
extern void HandleSignalLmDisconnectComplete(
                HCI_EV_DATA_DISCONNECT_COMPLETE_T *p_event_data);
extern bool HandleLEAdvMessage(LM_EV_ADVERTISING_REPORT_T* report);

#endif /* __APP_FW_EVENT_HANDLER_H__ */

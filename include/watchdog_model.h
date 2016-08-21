/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  CSRmesh is a product of Qualcomm Technologies International Ltd.
 */
/*! \file Watchdog_model.h
 *
 *  \brief Defines CSRmesh Watchdog Model specific data structures\n
 */
/******************************************************************************/

#ifndef __WATCHDOG_MODEL_H__
#define __WATCHDOG_MODEL_H__

#include "csr_mesh_model_common.h"

/*! \addtogroup Watchdog_Model 
 *  \brief Watchdog Model API
 *
 * <br>The Watchdog Model uses the concept of an interval that allows a device to send out a watchdog message at a known frequency. This message can be used by devices to determine whether the sending device is still connected to the network. For example, a window sensor that only transmits when it detects an alarm condition uses the watchdog message to inform other devices that it is still alive. The periodicity of these watchdog messages can be configured on each device, allowing devices that need shorter reporting intervals to be configured accordingly.<br><br> After these watchdog messages are sent, the devices can stay active for a short period of time for the capture of responses and for other devices to reconfigure these devices. This is most useful to allow devices that normally never listen for CSRmesh messages to be reconfigured or managed by devices. For example, a temperature sensor may only transmit the current room temperature, and send out a watchdog message once a day; when this single watchdog message is received, the sending device would stay active and another device could check the firmware version and possibly request the sending device to start to update its firmware. This enables the lowest possible power operation for the majority of the time whilst still allowing devices to be reconfigured within a known period of time.<br><br> Watchdog messages can also be used to ensure that a predetermined pattern of activity is maintained. This is done by issuing watchdog messages where the expected messages should be sent but for some reason are not sent, for example when a home owner is on vacation. This ensures that a similar traffic pattern is maintained and thus offers protection against traffic analysis and occupancy detection.<br><br> For example, a light switch may send out a few messages between 07:00 and 08:00 in the morning, and a few messages between 19:00 and 22:00. It can simulate this by sending watchdog messages that are sized similarly. It can even simulate the appropriate responses from devices by requesting a response with a similarly appropriate size. Devices can therefore protect against passive eavesdroppers monitoring the occupancy of a building from the quantity, size, or timing of messages.<br><br><h4>Client Behaviour</h4>If a device does not send any messages, but it should normally send some messages, for example because the building is currently unoccupied, the device may send one or more WATCHDOG_MESSAGE s to simulate the typical functionality of the device. The size of the WATCHDOG_MESSAGE s should be the same as the typical MCP messages that the device normally sends, and the RspSize should be set to the size of the typical responses to these messages. The MCP Dst field should also be set to the typical destination identifiers this device uses.<br><br> <h6>Note:</h6> This behaviour prevents a passive eavesdropper from performing coarse traffic analysis to determine if a building is occupied and the coarse location of people in this building<br><br><h4> Background Behaviour</h4>The device should transmit a new random WATCHDOG_MESSAGE at least once every Interval state value seconds. A random message is defined as one that has the RspSize field set to the value zero, and the RandomData field set to a random number of random values, from 0 to 9 octets in length.<br><br> The ActiveAfterTime state value determines for how long a device should listen for the CSRmesh packets after transmitting a WATCHDOG_MESSAGE. This allows devices to be reconfigured at least once every Interval by waiting for a WATCHDOG_MESSAGE to be received from that device and then sending them another message at this time.<br><br>A device may transmit a new random WATCHDOG_MESSAGE at similar intervals to the normal operation of the device.
 * @{
 */

/*============================================================================*
    Public Definitions
 *============================================================================*/

/*! \brief CSRmesh Watchdog Model message types */

/*! \brief Upon receiving a WATCHDOG_MESSAGE message, if the RspSize field is set to a non-zero value, then the device shall respond with a WATCHDOG_MESSAGE with the RspSize field set to zero, and RspSize -1 octets of additional RandomData. */
typedef struct
{
    CsrUint8 rspsize; /*!< \brief Size of any expected response */
    CsrUint8 randomdata[9/size_in_bytes(CsrUint8)]; /*!< \brief Random data from a device: Because the RandomData field is encrypted, a passive eavesdropper will not be able to determine the contents of this message, compared with any other message. However, it is still good practice to use random contents in this field. Note: All messages are encrypted and the sequence number is unique even if the RandomData field is a constant value, so the transmitted data are different in each message. However, knowledge of this constant value will enable a possible known plain text attack, so the use of a constant value for the RandomData field is not recommended. */
    CsrUint8 randomdata_len; /*!< \brief Length of the randomdata field */
} CSRMESH_WATCHDOG_MESSAGE_T;

/*! \brief Upon receiving a WATCHDOG_SET_INTERVAL message, the device shall save the Interval and ActiveAfterTime fields into the Interval and ActiveAfterTime variables and respond with a WATCHDOG_INTERVAL message with the current variable values. */
typedef struct
{
    CsrUint16 interval; /*!< \brief The watchdog Interval being set in seconds and indicates the maximum interval that watchdog messages should be sent. */
    CsrUint16 activeaftertime; /*!< \brief The watchdog ActiveAfterTime being set in seconds that indicates the period of time that a device will listen for responses after sending a watchdog message. */
    CsrUint8 tid; /*!< \brief Transaction ID. The TID or transaction identifier field is an 8-bit integer that identifies a given message with a known transaction from a source device. */
} CSRMESH_WATCHDOG_SET_INTERVAL_T;

/*! \brief Watchdog interval state */
typedef struct
{
    CsrUint16 interval; /*!< \brief The current watchdog Interval in seconds that contains the maximum interval that watchdog messages should be sent. */
    CsrUint16 activeaftertime; /*!< \brief The current watchdog ActiveAfterTime that contains the period of time that a device will listen for responses after sending a watchdog message. */
    CsrUint8 tid; /*!< \brief Transaction ID. The TID or transaction identifier field is an 8-bit integer that identifies a given message with a known transaction from a source device. */
} CSRMESH_WATCHDOG_INTERVAL_T;


/*!@} */
#endif /* __WATCHDOG_MODEL_H__ */


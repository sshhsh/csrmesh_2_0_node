/*****************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      csr_mesh_switch_hw.h
 *
 *  DESCRIPTION
 *      This file defines a interface to abstract hardware specifics of switch.
 *
 *****************************************************************************/

#ifndef __CSR_MESH_SWITCH_HW_H__
#define __CSR_MESH_SWITCH_HW_H__

#include "csr_mesh_switch.h"
/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function initializes the switch hardware, like PIO, interface etc. */
extern void SwitchHardwareInit(void);

/* PIO changed events for handling button presses */
extern void HandlePIOChangedEvent(uint32 pio_changed);

/* Controls the light device power. */
extern void LightHardwarePowerControl(bool power_on);

/* Controls the color of the light. */
extern bool LightHardwareSetColor(uint8 red, uint8 green, uint8 blue);

/* Controls the brightness of the White light. */
extern void LightHardwareSetLevel(uint8 level);

/* Controls the colour temperature. */
extern bool LightHardwareSetColorTemp(uint16 temp);

/* Controls the blink colour and duration of light. */
extern bool LightHardwareSetBlink(uint8 red, uint8 green, uint8 blue,
                                  uint8 on_time, uint8 off_time);
#endif /* __CSR_MESH_SWITCH_HW_H__ */


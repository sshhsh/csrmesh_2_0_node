/*****************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      csr_mesh_light_hw.h
 *
 *  DESCRIPTION
 *      This file defines a interface to abstract hardware specifics of light.
 *
 *****************************************************************************/

#ifndef __CSR_MESH_LIGHT_HW_H__
#define __CSR_MESH_LIGHT_HW_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>
#include <pio.h>
#include <timer.h>
#include <sys_events.h>

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/

/* This function initializes the light hardware, like PIO, interface etc. */
extern void LightHardwareInit(void);

/* Controls the light device power. */
extern void LightHardwarePowerControl(bool power_on);

/* Controls the color of the light. */
extern bool LightHardwareSetColor(uint8 red, uint8 green, uint8 blue);

/* Controls the brightness of the light. */
extern void LightHardwareSetLevel(uint8 red, uint8 green, uint8 blue, 
                                  uint8 level);

/* Controls the colour temperature. */
extern bool LightHardwareSetColorTemp(uint16 temp);

/* Controls the blink colour and duration of light. */
extern bool LightHardwareSetBlink(uint8 red, uint8 green, uint8 blue,
                                  uint8 on_time, uint8 off_time);
#ifdef COLOUR_TEMP_ENABLED
/* Get the RGB values for the passed color temperature values */
extern void LightHardwareGetRGBFromColorTemp(uint16 temp, uint8 *red, 
                                             uint8 *green, uint8 *blue);
#endif

#ifdef USE_ASSOCIATION_REMOVAL_KEY
extern void HandlePIOEvent(pio_changed_data *data);
#endif 
#endif /* __CSR_MESH_LIGHT_HW_H__ */


/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      iot_hw.h
 *
 *  DESCRIPTION
 *      This file defines all the function which interact with IOT hardware
 *      to control power, intensity and colour of RGB LEDs.
 *
 *  NOTES
 *
 ******************************************************************************/
#ifndef __IOT_HW_H__
#define __IOT_HW_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/

/* All the LED Blinking and Buzzer code has been put under these compiler flags
 * Disable these flags at the time of current consumption measurement 
 */

#define PIO_BIT_MASK(pio)       (0x01 << (pio))

/* PIO direction */
#define PIO_DIRECTION_INPUT     (FALSE)
#define PIO_DIRECTION_OUTPUT    (TRUE)

/* PIO state */
#define PIO_STATE_HIGH          (TRUE)
#define PIO_STATE_LOW           (FALSE)

#define LED_PWM_RED     (0)
#define LED_PWM_GREEN   (1)
#define LED_PWM_BLUE    (2)

#define LED_PIO_RED     (9)
#define LED_PIO_GREEN   (10)
#define LED_PIO_BLUE    (11)

#define SW2_PIO         (1)
#define SW3_PIO         (0)
#define SW4_PIO         (4)

#define SW2_MASK      PIO_BIT_MASK(SW2_PIO)
#define SW3_MASK      PIO_BIT_MASK(SW3_PIO)
#define SW4_MASK      PIO_BIT_MASK(SW4_PIO)

/* Bit-mask of all the Temperature Sensor PIOs used by the board. */
#define BUTTONS_BIT_MASK    (SW2_MASK | SW3_MASK | SW4_MASK)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
/* This function initialises the Red, Green and Blue LED lines. */
extern void IOTLightControlDeviceInit(void);

/* This function sets the brightness level for white light. */
extern void IOTLightControlDeviceSetLevel(uint8 level);

/* This function sets the colour as per RGB values. */
extern void IOTLightControlDeviceSetColor(uint8 red, uint8 green, uint8 blue);

/* This function sets the Power State of Light. */
extern void IOTLightControlDevicePower(bool power_on);

/* This function Initializes Switch GPIO */
extern void IOTSwitchInit(void);

/* This function sets colour and blink time for LEDs. */
extern void IOTLightControlDeviceBlink(uint8 red, uint8 green, uint8 blue,
                                       uint8 on_time, uint8 off_time);
#endif /*__IOT_HW_H__*/

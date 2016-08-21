/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      fast_pwm.h
 *
 *  DESCRIPTION
 *      This file illustrates how to use the PIO controller to produce fast
 *      PWM output. The PWM output has 1 ms intervals, and the pulse widths
 *      adjustable with 4 microsecond steps. PIOs 8~15 can be used as output.
 *      And the user can use other PIOs by chaining the PIO controller code,
 *      as long as they are in the same port.
 *
 *  NOTES
 *
 ******************************************************************************/

#ifndef __FAST_PWM_H__
#define __FAST_PWM_H__

/*============================================================================*
 *  Public Definitions
 *============================================================================*/
/* PIO numbers for PWM Ports */
#define PWM0_PORT  8
#define PWM1_PORT  9
#define PWM2_PORT  10
#define PWM3_PORT  11
#define PWM4_PORT  12
#define PWM5_PORT  13
#define PWM6_PORT  14
#define PWM7_PORT  15

/* Configures a PWM port. */
void PioFastPwmConfig(uint32 pio_mask);

/* Sets the bright/dull widths for a PWM port. */
bool PioFastPwmSetWidth(uint8 pwm_port, uint8 bright_width, uint8 dull_width,
                        bool inverted);

/* Enable the PWM. */
void PioFastPwmEnable(bool enable);

/* Sets the PWM Periods. */
void PioFastPwmSetPeriods(uint16 bright, uint16 dull);

#endif /* __FAST_PWM_H__ */

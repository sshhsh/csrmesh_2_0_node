/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  Application version 2.0
 *
 *  FILE
 *      fast_pwm.c
 *
 *  DESCRIPTION
 *      This file illustrates how to use the PIO controller to produce fast
 *      PWM output. The PWM output has approx. 1 ms intervals, and
 *      the pulse widths adjustable with 4 microsecond steps. PIOs 8~15 can
 *      be used as output. And the user can use other PIOs by chaining
 *      the PIO controller code, as long as they are in the same port.
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <pio.h>
#include <pio_ctrlr.h>
#include <sleep.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"
#include "fast_pwm.h"
#include "Iot_hw.h"

#ifdef ENABLE_FAST_PWM

/* Included externally in PIO controller code.*/
void pio_ctrlr_code(void);
/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/
/*----------------------------------------------------------------------------*
 *  NAME
 *      PioFastPwmSetWidth
 *
 *  DESCRIPTION
 *      This function sets the required pulse width in multiples of 4us
 *      on a PWM port.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
 bool PioFastPwmSetWidth(uint8 pwm_port, uint8 bright_width, uint8 dull_width,
                        bool inverted)
{
    if(pwm_port < PWM0_PORT || pwm_port > PWM7_PORT ||
       bright_width > 255 || dull_width > 255)
        return FALSE;

    uint16*address=PIO_CONTROLLER_DATA_WORD+((pwm_port-PWM0_PORT)>>1);

    if(pwm_port&1)
    {
        *address&=0x00ff;
        *address|=(bright_width<<8);
        address+=4;
        *address&=0x00ff;
        *address|=(dull_width<<8);
    }
    else
    {
        *address&=0xff00;
        *address|=bright_width;
        address+=4;
        *address&=0xff00;
        *address|=dull_width;
    }

    address=PIO_CONTROLLER_DATA_WORD+8;

    if(inverted)
        *address&=~(1<<(pwm_port-PWM0_PORT));
    else
        *address|=1<<(pwm_port-PWM0_PORT);
    return TRUE;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      PioFastPwmSetPeriods
 *
 *  DESCRIPTION
 *      This function sets bright and dull periods for PWM. This is
 *      applicable for all PWM ports enabled.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void PioFastPwmSetPeriods(uint16 bright, uint16 dull)
{
    *(PIO_CONTROLLER_DATA_WORD+9)=bright;
    *(PIO_CONTROLLER_DATA_WORD+10)=dull;
    *(PIO_CONTROLLER_DATA_WORD+11)=1; /* reset */
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      PioFastPwmConfig
 *
 *  DESCRIPTION
 *      This function selects the PWM ports to be configured.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void PioFastPwmConfig(uint32 pio_mask)
{
    PioSetModes(pio_mask,  pio_mode_pio_controller);
    //PioSetPullModes(pio_mask, pio_mode_no_pulls);
	PioSetPullModes(pio_mask, pio_mode_weak_pull_down);

    PioCtrlrInit((uint16*)&pio_ctrlr_code);
    PioCtrlrClock(TRUE);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      PioFastPwmEnable
 *
 *  DESCRIPTION
 *      This function enables/disables PWM.
 *
 *  RETURNS
 *      Nothing.
 *
 *----------------------------------------------------------------------------*/
void PioFastPwmEnable(bool enable)
{
    if(enable)
    {
    	PioSet(LED_PIO_EN, 1);
        SleepModeChange(sleep_mode_shallow);
        PioCtrlrStart();
    }
    else
    {
    	PioSet(LED_PIO_EN, 0);
        PioCtrlrStop();
        SleepModeChange(sleep_mode_deep);
    }
}
#endif /* ENABLE_FAST_PWM */
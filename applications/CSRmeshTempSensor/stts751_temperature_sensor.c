/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      stts751_temperature_sensor.c
 *
 *  DESCRIPTION
 *      This file implements the procedure for communicating with a STTS751
 *      Sensor.
 *
 *
 *****************************************************************************/

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>
#include <macros.h>

/*============================================================================*
 *  Local Header Files
 *============================================================================*/
#include "user_config.h"
#include "stts751_temperature_sensor.h"
#include "i2c_comms.h"
#include "app_debug.h"

#ifdef TEMPERATURE_SENSOR_STTS751
/*============================================================================*
 *  Public Function Implementations
 *============================================================================*/
/*----------------------------------------------------------------------------*
 *  NAME
 *      STTS751_Init
 *
 *  DESCRIPTION
 *      This function initialises the temperature sensor STTS751
 *
 *  RETURNS
 *      TRUE if success.
 *
 *---------------------------------------------------------------------------*/
extern bool STTS751_Init(void)
{
    uint8 manu_id = 0;
    bool  success = FALSE;

    /* Acquire I2C bus */
    if(I2CAcquire())
    {
        /* Initialise I2C communication. */
        I2CcommsInit();

        /* Read the Manufacturer's ID and check */
        success = I2CReadRegister(STTS751_I2C_ADDRESS, REG_MANU_ID, &manu_id);
        if ((!success) || (STTS751_MANU_ID != manu_id))
        {
            success = FALSE;
        }
        else
        {
            /* Configure the temperature sensor
             * - Standby mode for one shot readings
             * - 12 - bit resolution for 0.0625 degrees Centigrade accuracy.
             */
            success = I2CWriteRegister(STTS751_I2C_ADDRESS, REG_CONFIG,
                                       (STANDBY_MODE|TRES_12_BITS));
        }

        /* Release the I2C bus */
        I2CRelease();
    }
    return success;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      STTS751_Calibration
 *
 *  DESCRIPTION
 *      This function calibrates the temperature sensor STTS751.
 *
 *  RETURNS
 *      Nothing.
 *
 *---------------------------------------------------------------------------*/
extern void STTS751_Calibration(void)
{
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      STTS751_InitiateOneShotRead
 *
 *  DESCRIPTION
 *      This function initiates Read from the temperature sensor STTS751.
 *
 *  RETURNS
 *      TRUE if read succeeds.
 *
 *----------------------------------------------------------------------------*/
extern bool STTS751_InitiateOneShotRead(void)
{
    bool success = FALSE;

    if (I2CAcquire())
    {
        /* Initialise I2C. */
        I2CcommsInit();

        /* Write to One Shot Register. */
        success = I2CWriteRegister(STTS751_I2C_ADDRESS, REG_ONE_SHOT_READ, 0x1);

        /* Release the I2C bus */
        I2CRelease();
    }

    return success;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      STTS751_ReadTemperature
 *
 *  DESCRIPTION
 *      This function read the temperature from sensor STTS751.
 *
 *  RETURNS
 *      TRUE if read succeeds.
 *
 *----------------------------------------------------------------------------*/
extern bool STTS751_ReadTemperature(int16 *temp)
{
    bool  success = FALSE;
    uint8 status  = 0x00;
    uint8 integer, fraction;

    /* Set temperature to Invalid value */
    *temp = INVALID_TEMPERATURE;

    if (I2CAcquire())
    {
        /* Initialise I2C. */
        I2CcommsInit();

        /* Check if conversion is in progress. */
        success = I2CReadRegister(STTS751_I2C_ADDRESS, REG_STATUS, &status);

        /* If read is successful and conversion is not in progress,
         * Read the temperature from the device.
         */
        if ((success) && (!(status & STATUS_BUSY_BITMASK)))
        {
            /* Read the temperature. */
            success  = 
                I2CReadRegister(STTS751_I2C_ADDRESS, REG_TEMP_MSB, &integer);
            success &= 
                I2CReadRegister(STTS751_I2C_ADDRESS, REG_TEMP_LSB, &fraction);

            if (success)
            {
                /* Sign extend the temperature value. */
                if (integer & 0x80) integer |= 0xFF00;

                /* Fill the Integer part into temperature value. */
                *temp = (int16)((uint16)integer << TRES_BITS_SHIFT(12));

                /* Right Shift the Fractional part as per resolution. */
                fraction = (fraction & 0xFF) >> TRES_BITS_SHIFT(12);

                *temp |= fraction;
            }
        }

        /* Release the I2C bus */
        I2CRelease();
    }

    return success;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      STTS751_ReadCallback
 *
 *  DESCRIPTION
 *      This function implements the callback function for read operation.
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/
extern void STTS751_ReadCallback(void)
{
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      STTS751_ShutDown
 *
 *  DESCRIPTION
 *      This function shuts down the temperature sensor STTS751
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/
extern void STTS751_ShutDown(void)
{
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      STTS751_InterruptHandler
 *
 *  DESCRIPTION
 *      This function handles the interrupt from temperature sensor STTS751
 *
 *  RETURNS
 *      Nothing
 *
 *---------------------------------------------------------------------------*/
extern void STTS751_InterruptHandler(void)
{
}

#endif /* TEMPERATURE_SENSOR_STTS751 */


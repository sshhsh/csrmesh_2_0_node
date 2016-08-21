/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  %%appversion
 *
 *  FILE
 *      stts751_temperature_sensor.h
 *
 *  DESCRIPTION
 *      Header file for STTS751 Temperature Sensor
 *
 *****************************************************************************/

#ifndef __STTS751_TEMPERATURE_SENSOR_H__
#define __STTS751_TEMPERATURE_SENSOR_H__

/*============================================================================*
 *  SDK Header Files
 *============================================================================*/
#include <types.h>

/*============================================================================*
 *  local Header Files
 *============================================================================*/
#include "user_config.h"
#include "i2c_comms.h"

#ifdef TEMPERATURE_SENSOR_STTS751
/*============================================================================*
 *  Public Definitions
 *============================================================================*/
/* Temperature sensor STTS751 specific definitions */
#define STTS751_0_I2C_ADDRESS           (0x72)
#define STTS751_1_I2C_ADDRESS           (0x76)

#define STTS751_I2C_ADDRESS             (STTS751_0_I2C_ADDRESS)

/* Manufacturer ID for STTS751. */
#define STTS751_MANU_ID                 (0x53)

/* STTS751 I2C registers */
#define REG_TEMP_MSB                    (0x00)
#define REG_STATUS                      (0x01)
#define REG_TEMP_LSB                    (0x02)
#define REG_CONFIG                      (0x03)
#define REG_CONV_RATE                   (0x04)
#define REG_TEMP_HI_LIM_MSB             (0x05)
#define REG_TEMP_HI_LIM_LSB             (0x06)
#define REG_TEMP_LO_LIM_MSB             (0x07)
#define REG_TEMP_LO_LIM_LSB             (0x08)
#define REG_ONE_SHOT_READ               (0x0F)
#define REG_THERM_LIM                   (0x20)
#define REG_THERM_HYS                   (0x21)
#define REG_I2C_TIMEOUT                 (0x22)
#define REG_PID                         (0xFD)
#define REG_MANU_ID                     (0xFE)
#define REG_REVISION                    (0xFF)

/* Configuration register bit fields */
/* Enable/disable event assertion in out of bounds
 * condition
 */
#define STTS751_ENABLE_EVENT            (0x00)
#define STTS751_DISABLE_EVENT           (0x80)

/* Mode */
#define CONTINUOUS_CONV                 (0x00)
#define STANDBY_MODE                    (0x40)

/* Temperature resolution bits */
#define TRES_12_BITS                    (0x03 << 3)
#define TRES_11_BITS                    (0x01 << 3)
#define TRES_10_BITS                    (0x00 << 3)
#define TRES_09_BITS                    (0x02 << 3)

/* Fractional Mask depending upon the resolution */
#define TRES_09_BITS_FRACTIONAL_MASK    (0x80)
#define TRES_10_BITS_FRACTIONAL_MASK    (0xC0)
#define TRES_11_BITS_FRACTIONAL_MASK    (0xE0)
#define TRES_12_BITS_FRACTIONAL_MASK    (0xF0)

/* Right Shift value for Fractional Part.
 * Resolution      : 9/10/11/12 bits.
 * Integer Part    : 8-bits.
 * Fractional Part : Resolution - Integer Part
 */
#define TRES_BITS_SHIFT(res)            (16 - (res))

/* Status bits  */
#define STATUS_BUSY_BITMASK             (0x80)
#define STATUS_TEMP_HI_BITMASK          (0x40)
#define STATUS_TEMP_LO_BITMASK          (0x20)

/* Maximum conversion time in milliseconds
 * Conversion times for the STTS751 sensor
 *     9-bit Typical: 10.5ms Max: 14 ms
 *    10-bit Typical: 21ms   Max: 28 ms
 *    11-bit Typical: 42ms   Max: 56 ms
 *    12-bit Typical: 84ms   Max: 112 ms
 */
#define MAX_CONVERSION_TIME             (112)

/* Invalid temperature. */
#define INVALID_TEMPERATURE             ((int16)0xFFFF)

/*============================================================================*
 *  Public Function Prototypes
 *============================================================================*/
/* This function initialises the temperature sensor STTS751 */
extern bool STTS751_Init(void);

/* This function calibrates the temperature sensor STTS751. */
extern void STTS751_Calibration(void);

/* This function initiates a single read temperature transaction. */
extern bool STTS751_InitiateOneShotRead(void);

/* This function reads data from the temperature sensor STTS751 */
extern bool STTS751_ReadTemperature(int16 *temp);

/* This function implements the callback function for read operation.*/
extern void STTS751_ReadCallback(void);

/* This function shuts down the temperature sensor STTS751 */
extern void STTS751_ShutDown(void);

/* This function handles the interrupt from temperature sensor STTS751 */
extern void STTS751_InterruptHandler(void);

#endif /* TEMPERATURE_SENSOR_STTS751 */
#endif /* __STTS751_TEMPERATURE_SENSOR_H__ */


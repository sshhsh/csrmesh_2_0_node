/******************************************************************************
 *  Copyright 2015 Qualcomm Technologies International, Ltd.
 *  Bluetooth Low Energy CSRmesh 2.0
 *  CSRmesh is a product of Qualcomm Technologies International Ltd."
 */
/*! \file sensor_types.h
 * 
 *  \brief Defines the types used in the CSRmesh Sensor and actuator Model 
 */
/*****************************************************************************/    
#ifndef __SENSOR_TYPES_H__
#define __SENSOR_TYPES_H__

/*! \addtogroup Sensor_Model
 * @{
 */

/*============================================================================*
Public Definitions
*============================================================================*/

/*! \brief CSRmesh Sensor Type */
typedef enum
{
    sensor_type_invalid = 0, /*!< \brief type_undefined */
    sensor_type_internal_air_temperature = 1, /*!< \brief temperature_kelvin */
    sensor_type_external_air_temperature = 2, /*!< \brief temperature_kelvin_range */
    sensor_type_desired_air_temperature = 3, /*!< \brief temperature_kelvin */
    sensor_type_internal_humidity = 4, /*!< \brief percentage */
    sensor_type_external_humidity = 5, /*!< \brief percentage */
    sensor_type_external_dewpoint = 6, /*!< \brief temperature_kelvin */
    sensor_type_internal_door = 7, /*!< \brief door_state */
    sensor_type_external_door = 8, /*!< \brief door_state */
    sensor_type_internal_window = 9, /*!< \brief window_state */
    sensor_type_external_window = 10, /*!< \brief window_state */
    sensor_type_solar_energy = 11, /*!< \brief watts_per_square_metre */
    sensor_type_number_of_activations = 12, /*!< \brief 16_bit_counter */
    sensor_type_fridge_temperature = 13, /*!< \brief temperature_kelvin */
    sensor_type_desired_fridge_temperature = 14, /*!< \brief temperature_kelvin */
    sensor_type_freezer_temperature = 15, /*!< \brief temperature_kelvin */
    sensor_type_desired_freezer_temperature = 16, /*!< \brief temperature_kelvin */
    sensor_type_oven_temperature = 17, /*!< \brief temperature_kelvin */
    sensor_type_desired_oven_temperature = 18, /*!< \brief temperature_kelvin */
    sensor_type_seat_occupied = 19, /*!< \brief seat_state */
    sensor_type_washing_machine_state = 20, /*!< \brief appliance_state */
    sensor_type_dish_washer_state = 21, /*!< \brief appliance_state */
    sensor_type_clothes_dryer_state = 22, /*!< \brief appliance_state */
    sensor_type_toaster_state = 23, /*!< \brief appliance_state */
    sensor_type_carbon_dioxide = 24, /*!< \brief parts_per_million */
    sensor_type_carbon_monoxide = 25, /*!< \brief parts_per_million */
    sensor_type_smoke = 26, /*!< \brief micrograms_per_cubic_metre */
    sensor_type_water_level = 27, /*!< \brief percentage */
    sensor_type_hot_water_temperature = 28, /*!< \brief temperature_kelvin */
    sensor_type_cold_water_temperature = 29, /*!< \brief temperature_kelvin */
    sensor_type_desired_water_temperature = 30, /*!< \brief temperature_kelvin */
    sensor_type_cooker_hob_back_left_state = 31, /*!< \brief cooker_hob_state */
    sensor_type_desired_cooker_hob_back_left_state = 32, /*!< \brief cooker_hob_state */
    sensor_type_cooker_hob_front_left_state = 33, /*!< \brief cooker_hob_state */
    sensor_type_desired_cooker_hob_front_left_state = 34, /*!< \brief cooker_hob_state */
    sensor_type_cooker_hob_back_middle_state = 35, /*!< \brief cooker_hob_state */
    sensor_type_desired_cooker_hob_back_middle_state = 36, /*!< \brief cooker_hob_state */
    sensor_type_cooker_hob_front_middle_state = 37, /*!< \brief cooker_hob_state */
    sensor_type_desired_cooker_hob_front_middle_state = 38, /*!< \brief cooker_hob_state */
    sensor_type_cooker_hob_back_right_state = 39, /*!< \brief cooker_hob_state */
    sensor_type_desired_cooker_hob_back_right_state = 40, /*!< \brief cooker_hob_state */
    sensor_type_cooker_hob_front_right_state = 41, /*!< \brief cooker_hob_state */
    sensor_type_desired_cooker_hob_front_right_state = 42, /*!< \brief cooker_hob_state */
    sensor_type_desired_wakeup_alarm_time = 43, /*!< \brief minutes_of_the_day */
    sensor_type_desired_second_wakeup_alarm_time = 44, /*!< \brief minutes_of_the_day */
    sensor_type_passive_infrared_state = 45, /*!< \brief movement_state */
    sensor_type_water_flowing = 46, /*!< \brief water_flow_rate */
    sensor_type_desired_water_flow = 47, /*!< \brief water_flow_rate */
    sensor_type_audio_level = 48, /*!< \brief decibel */
    sensor_type_desired_audio_level = 49, /*!< \brief decibel */
    sensor_type_fan_speed = 50, /*!< \brief percentage */
    sensor_type_desired_fan_speed = 51, /*!< \brief forward_backward */
    sensor_type_wind_speed = 52, /*!< \brief centimetres_per_second */
    sensor_type_wind_speed_gust = 53, /*!< \brief centimetres_per_second */
    sensor_type_wind_direction = 54, /*!< \brief direction */
    sensor_type_wind_direction_gust = 55, /*!< \brief direction */
    sensor_type_rain_fall_last_hour = 56, /*!< \brief millimetres */
    sensor_type_rain_fall_today = 57, /*!< \brief millimetres */
    sensor_type_barometric_pressure = 58, /*!< \brief air_pressure */
    sensor_type_soil_temperature = 59, /*!< \brief temperature_kelvin */
    sensor_type_soil_moisture = 60, /*!< \brief percentage */
    sensor_type_window_cover_position = 61, /*!< \brief percentage */
    sensor_type_desired_window_cover_position = 62, /*!< \brief percentage */
} sensor_type_t;

/*! \brief CSRmesh Temperature format. */
typedef uint16 SENSOR_FORMAT_TEMPERATURE_T;

/*!@} */
#endif /* __SENSOR_TYPES_H__ */

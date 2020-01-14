/**
 * @file vl53l1x.h
 *
 * @brief User functions to deal with ST's VL53L1 API
 *
 * @author Lucas Schneider <lucas.schneider@thunderatz.org>
 * @author Bernardo Coutinho <bernardo.coutinho@thunderatz.org>
 *
 * @date 01/2020
 */

#ifndef __VL53L1X_H__
#define __VL53L1X_H__

#include "vl53l1_api.h"
#include "vl53l1_platform.h"

/*****************************************
 * Public Constants
 *****************************************/

#define VL53L1_DEFAULT_ADDRESS 0x52

/*****************************************
 * Public Functions Prototypes
 *****************************************/

/**
 * @brief Initializes VL53L1 device.
 *
 * @param p_device        Pointer to the device handle.
 * @param p_calibration   Pointer to the device calibration data.
 *
 * @return Error code.
 * @retval VL53L1_ERROR_NONE   Success.
 * @retval "Other error code"   More details in @ref VL53L1_Error.
 */
VL53L1_Error vl53l1_init(VL53L1_Dev_t* p_device, VL53L1_CalibrationData_t* p_calibration);

/**
 * @brief Turns off device with the XSHUT pin.
 *
 * @param p_device      Pointer to the device handle.
 */
void vl53l1_turn_off(VL53L1_Dev_t* p_device);

/**
 * @brief Turns on device with the XSHUT pin and reads the model ID.
 *
 * @param p_device      Pointer to the device handle.
 */
void vl53l1_turn_on(VL53L1_Dev_t* p_device);

/**
 * @brief Updates device reading.
 *
 * @param p_device          Pointer to the device handle.
 * @param p_ranging_data    Pointer to device measurement data.
 * @param p_reading_mm      Pointer to the reading variable; measurement in millimeters.
 * @param max_range_mm      Device's empirical maximum range in millimeters.
 *
 * @return Error code.
 * @retval 0                         Success.
 * @retval "Number between 1 and 5"  Range status from sensor reading @see API_User_Manual.
 * @retval "Other error code"        More details in @ref VL53L1_Error.
 */
uint8_t vl53l1_update_reading(VL53L1_Dev_t* p_device, VL53L1_RangingMeasurementData_t* p_ranging_data,
                              uint16_t* p_reading_mm, uint16_t max_range_mm);

uint8_t check_API_status(); // NOT IMPLEMENTED YET

uint8_t vl53l1_reinit(); // NOT IMPLEMENTED YET

#endif // __VL53L1X_H__

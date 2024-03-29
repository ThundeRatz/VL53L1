/**
 * @file vl53l1.c
 *
 * @brief User functions to deal with ST's VL53L1 API
 *
 * @author Lucas Schneider <lucas.schneider@thunderatz.org>
 * @author Bernardo Coutinho <bernardo.coutinho@thunderatz.org>
 *
 * @date 02/2020
 */

#include "vl53l1.h"
#include "gpio.h"

/*****************************************
 * Private Macro
 *****************************************/

/**
 * @brief Returns minimum value between x and y.
 */
#define min(x, y) ((x) < (y) ? (x) : (y))

/*****************************************
 * Private Constants
 *****************************************/

#define DEFAULT_DISTANCEMODE VL53L1_DISTANCEMODE_LONG
#define SIGMA_LIMIT_VALUE_MM 50
#define SIGNAL_RATE_LIMIT_VALUE_MCPS 0.25
#define MEASUREMENT_TIMING_BUDGET_US 50000

/**
 * @brief Filter values, depending on range status
 */
#define VALID_RANGE_FILTER 0.8
#define SIGMA_FAIL_FILTER 0.05
#define SIGNAL_FAIL_FILTER 0.05
#define OUT_OF_BOUNDS_FILTER 0.3
#define DEFAULT_FILTER 0.4

/*****************************************
 * Public Functions Bodies Definitions
 *****************************************/

VL53L1_Error vl53l1_init(VL53L1_Dev_t* p_device, VL53L1_CalibrationData_t* p_calibration) {
    VL53L1_Error Status = VL53L1_ERROR_NONE;

    // Wait Device Booted
    Status = VL53L1_WaitDeviceBooted(p_device);

    if (Status == VL53L1_ERROR_NONE) {
        // Data initialization
        Status = VL53L1_DataInit(p_device);
    }

    if (Status == VL53L1_ERROR_NONE) {
        // Device initialization
        Status = VL53L1_StaticInit(p_device);
    }

    if (p_device->calibrated) {
        // Load calibration data
        if (Status == VL53L1_ERROR_NONE) {
            Status = VL53L1_SetCalibrationData(p_device, p_calibration);
        }
    } else {
        // Device calibration
        if (Status == VL53L1_ERROR_NONE) {
            Status = VL53L1_PerformRefSpadManagement(p_device);
        }

        // Saving device calibration data
        if (Status == VL53L1_ERROR_NONE) {
            Status = VL53L1_GetCalibrationData(p_device, p_calibration);
        }
    }

    if (Status == VL53L1_ERROR_NONE) {
        // Set Distance Mode as Medium
        Status = VL53L1_SetDistanceMode(p_device, p_device->distance_mode);
    }

    // Enable Sigma Signal and Threshold check
    if (Status == VL53L1_ERROR_NONE) {
        Status = VL53L1_SetLimitCheckEnable(p_device,
                                            VL53L1_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
    }

    if (Status == VL53L1_ERROR_NONE) {
        Status = VL53L1_SetLimitCheckEnable(p_device,
                                            VL53L1_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
    }

    // Set Values
    if (Status == VL53L1_ERROR_NONE) {
        // Increasing Sigma increases maximum range
        Status = VL53L1_SetLimitCheckValue(p_device,
                                           VL53L1_CHECKENABLE_SIGMA_FINAL_RANGE,
                                           (FixPoint1616_t) (SIGMA_LIMIT_VALUE_MM * (1 << 16)));
    }

    if (Status == VL53L1_ERROR_NONE) {
        // Decreasing Signal Rate increases maximum range
        Status = VL53L1_SetLimitCheckValue(p_device,
                                           VL53L1_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
                                           (FixPoint1616_t) (SIGNAL_RATE_LIMIT_VALUE_MCPS * (1 << 16)));
    }

    if (Status == VL53L1_ERROR_NONE) {
        // Set Timing Budget - Time to perform one measurement
        // Higher Timing Budget means more accuracy and higher maximum range
        Status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(p_device, p_device->timing_budget_us);
    }

    if (Status == VL53L1_ERROR_NONE) {
        // Set Inter-Measurement Period - Delay between two operations
        Status = VL53L1_SetInterMeasurementPeriodMilliSeconds(p_device, p_device->timing_budget_us/1000 + 5);
    }

    // Start reading
    if (Status == VL53L1_ERROR_NONE) {
        Status = VL53L1_ClearInterruptAndStartMeasurement(p_device);
    }

    VL53L1_WaitMeasurementDataReady(p_device);

    VL53L1_ClearInterruptAndStartMeasurement(p_device);

    return Status;
}

void vl53l1_turn_off(VL53L1_Dev_t* p_device) {
    HAL_GPIO_WritePin(p_device->xshut_port, p_device->xshut_pin, GPIO_PIN_RESET);
}

void vl53l1_turn_on(VL53L1_Dev_t* p_device) {
    uint16_t wordData;
    HAL_GPIO_WritePin(p_device->xshut_port, p_device->xshut_pin, GPIO_PIN_SET);
    VL53L1_RdWord(p_device, VL53L1_IDENTIFICATION__MODEL_ID, &wordData);
}

void vl53l1_set_default_config(VL53L1_Dev_t* p_device) {
    p_device->I2cDevAddr = VL53L1_DEFAULT_ADDRESS;
    p_device->comms_type = 1;  // I2C
    p_device->present = 0;
    p_device->calibrated = 0;
    p_device->distance_mode = DEFAULT_DISTANCEMODE;
    p_device->timing_budget_us = MEASUREMENT_TIMING_BUDGET_US;
}

uint8_t vl53l1_update_reading(VL53L1_Dev_t* p_device, VL53L1_RangingMeasurementData_t* p_ranging_data,
                              uint16_t* p_reading_mm, uint16_t max_range_mm) {
    uint8_t status = 0;
    uint16_t aux_range = *p_reading_mm;
    status = VL53L1_GetRangingMeasurementData(p_device, p_ranging_data);

    if (status != VL53L1_ERROR_NONE) {
        return status;
    }

    uint8_t range_status = p_ranging_data->RangeStatus;

    if (range_status == 0) {
        // VALID RANGE
        aux_range = (p_ranging_data->RangeMilliMeter) * VALID_RANGE_FILTER + (1 - VALID_RANGE_FILTER) * aux_range;
    } else if (range_status == 1) {
        // SIGMA FAIL
        aux_range = (p_ranging_data->RangeMilliMeter) * SIGMA_FAIL_FILTER + (1 - SIGMA_FAIL_FILTER) * aux_range;
    } else if (range_status == 2) {
        // SIGNAL FAIL
        aux_range = (p_ranging_data->RangeMilliMeter) * SIGNAL_FAIL_FILTER + (1 - SIGNAL_FAIL_FILTER) * aux_range;
    } else if (range_status == 4) {
        // OUT OF BOUNDS FAIL
        aux_range = max_range_mm * OUT_OF_BOUNDS_FILTER + (1 - OUT_OF_BOUNDS_FILTER) * aux_range;
    } else if (range_status == 5) {
        // HARDWARE FAIL
        aux_range = max_range_mm;
    } else {
        // 7 - WRAP TARGET FAIL
        aux_range = (p_ranging_data->RangeMilliMeter) * DEFAULT_FILTER + (1 - DEFAULT_FILTER) * aux_range;
    }

    *p_reading_mm = min(aux_range, max_range_mm);

    if (status == 0) {
        status = range_status;
    }

    VL53L1_ClearInterruptAndStartMeasurement(p_device);

    return status;
}

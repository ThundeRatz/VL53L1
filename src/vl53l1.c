/**
 * @file vl53l1.c
 *
 * @brief User functions to deal with ST's VL53L1 API
 *
 * @author Lucas Schneider <lucas.schneider@thunderatz.org>
 * @author Bernardo Coutinho <bernardo.coutinho@thunderatz.org>
 *
 * @date 01/2020
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

#define DISTANCEMODE VL53L1_DISTANCEMODE_MEDIUM
#define SIGMA_LIMIT_VALUE_MM 18
#define SIGNAL_RATE_LIMIT_VALUE_MCPS 0.25
#define MEASUREMENT_TIME_BUDGET_US 50000

/**
 * @brief Filter values, depending on range status
 */
#define VALID_RANGE_FILTER 0.8
#define SIGMA_FAIL_FILTER 0.2
#define DEFAULT_FILTER 0.4

/*****************************************
 * Public Functions Bodies Definitions
 *****************************************/

VL53L1_Error vl53l1_init(VL53L1_Dev_t* p_device, VL53L1_DeviceInfo_t device_info,
                         VL53L1_CalibrationData_t* p_calibration) {
    VL53L1_Error Status = VL53L1_ERROR_NONE;

    // Wait Device Booted
    Status = VL53L1_WaitDeviceBooted(p_device);

    if (Status == VL53L1_ERROR_NONE) {
        // Data initialization
        Status = VL53L1_DataInit(p_device);
    }

    if (Status == VL53L1_ERROR_NONE) {
        Status = VL53L1_GetDeviceInfo(p_device, &device_info);

        if (Status == VL53L1_ERROR_NONE) {
            if ((device_info.ProductRevisionMinor != 1) && (device_info.ProductRevisionMinor != 1)) {
                Status = VL53L1_ERROR_NOT_SUPPORTED;
            }
        }
    }

    if (Status == VL53L1_ERROR_NONE) {
        // Device Initialization
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
        Status = VL53L1_SetDistanceMode(p_device, DISTANCEMODE);
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
        // Setting Timing Budget
        Status = VL53L1_SetMeasurementTimingBudgetMicroSeconds(p_device, MEASUREMENT_TIME_BUDGET_US);
    }

    if (Status == VL53L1_ERROR_NONE) {
        Status = VL53L1_SetInterMeasurementPeriodMilliSeconds(p_device, MEASUREMENT_TIME_BUDGET_US / 1000 + 5);
    }

    // Start reading
    if (Status == VL53L1_ERROR_NONE) {
        Status = VL53L1_StartMeasurement(p_device);
    }

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
    } else if (range_status == 4) {
        // PHASE FAIL

        /* In this case, aux_range will not be updated, because the reading is mostly random when this erro occurs */
    } else if (range_status == 5) {
        // HARDWARE FAIL
        aux_range = max_range_mm;
    } else {
        // 2 - SIGNAL FAIL
        // 3 - MIN RANGE FAIL
        aux_range = (p_ranging_data->RangeMilliMeter) * DEFAULT_FILTER + (1 - DEFAULT_FILTER) * aux_range;
    }

    *p_reading_mm = min(aux_range, max_range_mm);

    if (status == 0) {
        status = range_status;
    }

    return status;
}

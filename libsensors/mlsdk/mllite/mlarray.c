/*
 $License:
   Copyright 2011 InvenSense, Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
  $
 */
/******************************************************************************
 *
 * $Id: mlarray.c 5085 2011-04-08 22:25:14Z phickey $
 *
 *****************************************************************************/

/**
 *  @defgroup ML
 *  @{
 *      @file   mlarray.c
 *      @brief  APIs to read different data sets from FIFO.
 */

/* ------------------ */
/* - Include Files. - */
/* ------------------ */
#include "ml.h"
#include "mltypes.h"
#include "mlinclude.h"
#include "mlMathFunc.h"
#include "mlmath.h"
#include "mlstates.h"
#include "mlFIFO.h"
#include "mlsupervisor.h"
#include "mldl.h"
#include "dmpKey.h"
#include "compass.h"

/**
 *  @brief  inv_get_gyro is used to get the most recent gyroscope measurement.
 *          The argument array elements are ordered X,Y,Z.
 *          The values are scaled at 1 dps = 2^16 LSBs.
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 3 cells long </b>.
 *
 *  @return Zero if the command is successful; an ML error code otherwise.
 */

 /* inv_get_gyro implemented in mlFIFO.c */

/**
 *  @brief  inv_get_accel is used to get the most recent
 *          accelerometer measurement.
 *          The argument array elements are ordered X,Y,Z.
 *          The values are scaled in units of g (gravity),
 *          where 1 g = 2^16 LSBs.
 *
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 3 cells long </b>.
 *
 *  @return Zero if the command is successful; an ML error code otherwise.
 */
 /* inv_get_accel implemented in mlFIFO.c */

/**
 *  @brief  inv_get_quaternion is used to get the quaternion representation
 *          of the current sensor fusion solution.
 *          The values are scaled where 1.0 = 2^30 LSBs.
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 4 cells long </b>.
 *
 *  @return INV_SUCCESS if the command is successful; an ML error code otherwise.
 */
 /* inv_get_quaternion implemented in mlFIFO.c */

/**
 *  @brief  inv_get_linear_accel is used to get an estimate of linear
 *          acceleration, based on the most recent accelerometer measurement
 *          and sensor fusion solution.
 *          The values are scaled where 1 g (gravity) = 2^16 LSBs.
 *
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 3 cells long </b>.
 *
 *  @return Zero if the command is successful; an ML error code otherwise.
 */
 /* inv_get_linear_accel implemented in mlFIFO.c */

/**
 *  @brief  inv_get_gravity is used to get an estimate of the body frame
 *          gravity vector, based on the most recent sensor fusion solution.
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 3 cells long</b>.
 *
 *  @return Zero if the command is successful; an ML error code otherwise.
 */
 /* inv_get_gravity implemented in mlFIFO.c */

/**
 *  @brief  inv_get_gyro_float is used to get the most recent gyroscope measurement.
 *          The argument array elements are ordered X,Y,Z.
 *          The values are in units of dps (degrees per second).
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 3 cells long</b>.
 *
 *  @return INV_SUCCESS if the command is successful; an error code otherwise.
 */
inv_error_t inv_get_gyro_float(float *data)
{
    INVENSENSE_FUNC_START;

    inv_error_t result = INV_SUCCESS;
    long ldata[3];

    if (inv_get_state() < INV_STATE_DMP_OPENED)
        return INV_ERROR_SM_IMPROPER_STATE;

    if (NULL == data) {
        return INV_ERROR_INVALID_PARAMETER;
    }
    result = inv_get_gyro(ldata);
    data[0] = (float)ldata[0] / 65536.0f;
    data[1] = (float)ldata[1] / 65536.0f;
    data[2] = (float)ldata[2] / 65536.0f;

    return result;
}

/**
 *  @brief  inv_get_accel_float is used to get the most recent accelerometer measurement.
 *          The argument array elements are ordered X,Y,Z.
 *          The values are in units of g (gravity).
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 3 cells long</b>.
 *
 *  @return INV_SUCCESS if the command is successful; an error code otherwise.
 */
 /* inv_get_accel_float implemented in mlFIFO.c */

/**
 *  @brief  inv_get_rot_mat_float is used to get an array of nine data points representing the rotation
 *          matrix generated from all available sensors.
 *          The array format will be R11, R12, R13, R21, R22, R23, R31, R32,
 *          R33, representing the matrix:
 *          <center>R11 R12 R13</center>
 *          <center>R21 R22 R23</center>
 *          <center>R31 R32 R33</center>
 *          <b>Please refer to the "9-Axis Sensor Fusion Application Note" document,
 *          section 7 "Sensor Fusion Output", for details regarding rotation
 *          matrix output</b>.
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 9 cells long at least</b>.
 *
 *  @return INV_SUCCESS if the command is successful; an error code otherwise.
 */
inv_error_t inv_get_rot_mat_float(float *data)
{
    INVENSENSE_FUNC_START;

    inv_error_t result = INV_SUCCESS;

    if (inv_get_state() < INV_STATE_DMP_OPENED)
        return INV_ERROR_SM_IMPROPER_STATE;

    if (NULL == data) {
        return INV_ERROR_INVALID_PARAMETER;
    }
    {
        long qdata[4], rdata[9];
        inv_get_quaternion(qdata);
        inv_quaternion_to_rotation(qdata, rdata);
        data[0] = (float)rdata[0] / 1073741824.0f;
        data[1] = (float)rdata[1] / 1073741824.0f;
        data[2] = (float)rdata[2] / 1073741824.0f;
        data[3] = (float)rdata[3] / 1073741824.0f;
        data[4] = (float)rdata[4] / 1073741824.0f;
        data[5] = (float)rdata[5] / 1073741824.0f;
        data[6] = (float)rdata[6] / 1073741824.0f;
        data[7] = (float)rdata[7] / 1073741824.0f;
        data[8] = (float)rdata[8] / 1073741824.0f;
    }

    return result;
}

/**
 *  @brief  inv_get_quaternion_float is used to get the quaternion representation
 *          of the current sensor fusion solution.
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 4 cells long</b>.
 *
 *  @return INV_SUCCESS if the command is successful; an ML error code otherwise.
 */
 /* inv_get_quaternion_float implemented in mlFIFO.c */

/**
 *  @brief  inv_get_linear_accel_float is used to get an estimate of linear
 *          acceleration, based on the most recent accelerometer measurement
 *          and sensor fusion solution.
 *          The values are in units of g (gravity).
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 3 cells long</b>.
 *
 *  @return INV_SUCCESS if the command is successful; an error code otherwise.
 */
inv_error_t inv_get_linear_accel_float(float *data)
{
    INVENSENSE_FUNC_START;

    inv_error_t result = INV_SUCCESS;
    long ldata[3];

    if (inv_get_state() < INV_STATE_DMP_OPENED)
        return INV_ERROR_SM_IMPROPER_STATE;

    if (NULL == data) {
        return INV_ERROR_INVALID_PARAMETER;
    }

    result = inv_get_linear_accel(ldata);
    data[0] = (float)ldata[0] / 65536.0f;
    data[1] = (float)ldata[1] / 65536.0f;
    data[2] = (float)ldata[2] / 65536.0f;

    return result;
}

/**
 *  @brief  inv_get_gravity_float is used to get an estimate of the body frame
 *          gravity vector, based on the most recent sensor fusion solution.
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 3 cells long at least</b>.
 *
 *  @return INV_SUCCESS if the command is successful; an error code otherwise.
 */
inv_error_t inv_get_gravity_float(float *data)
{
    INVENSENSE_FUNC_START;

    inv_error_t result = INV_SUCCESS;
    long ldata[3];

    if (inv_get_state() < INV_STATE_DMP_OPENED)
        return INV_ERROR_SM_IMPROPER_STATE;

    if (NULL == data) {
        return INV_ERROR_INVALID_PARAMETER;
    }
    result = inv_get_gravity(ldata);
    data[0] = (float)ldata[0] / 65536.0f;
    data[1] = (float)ldata[1] / 65536.0f;
    data[2] = (float)ldata[2] / 65536.0f;

    return result;
}

/**
 *  @cond MPL
 *  @brief  inv_get_magnetometer_float is used to get magnetometer data
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or MLDmpPedometerStandAloneOpen() \endif
 *          must have been called.
 *
 *  @param  data
 *              A pointer to an array to be passed back to the user.
 *              <b>Must be 3 cells long</b>.
 *
 *  @return INV_SUCCESS if the command is successful; an error code otherwise.
 *  @endcond
 */
inv_error_t inv_get_magnetometer_float(float *data)
{
    INVENSENSE_FUNC_START;

    inv_error_t result = INV_SUCCESS;

    if (inv_get_state() < INV_STATE_DMP_OPENED)
        return INV_ERROR_SM_IMPROPER_STATE;

    if (NULL == data) {
        return INV_ERROR_INVALID_PARAMETER;
    }

    data[0] = (float)inv_obj.compass_calibrated_data[0] / 65536.0f;
    data[1] = (float)inv_obj.compass_calibrated_data[1] / 65536.0f;
    data[2] = (float)inv_obj.compass_calibrated_data[2] / 65536.0f;

    return result;
}

/**
 * Returns the curren compass accuracy.
 *
 * - 0: Unknown: The accuracy is unreliable and compass data should not be used
 * - 1: Low: The compass accuracy is low.
 * - 2: Medium: The compass accuracy is medium.
 * - 3: High: The compas acurracy is high and can be trusted
 *
 * @param accuracy The accuracy level in the range 0-3
 *
 * @return ML_SUCCESS or non-zero error code
 */
inv_error_t inv_get_compass_accuracy(int *accuracy)
{
    if (inv_get_state() != INV_STATE_DMP_STARTED)
        return INV_ERROR_SM_IMPROPER_STATE;

    *accuracy = inv_obj.compass_accuracy;
    return INV_SUCCESS;
}

/**
 *  @brief  inv_set_accel_bias is used to set the accelerometer bias.
 *          The argument array elements are ordered X,Y,Z.
 *          The values are scaled in units of g (gravity),
 *          where 1 g = 2^16 LSBs.
 *
 *          Please refer to the provided "9-Axis Sensor Fusion
 *          Application Note" document provided.
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or
 *          MLDmpPedometerStandAloneOpen() \endif
 *
 *  @param  data        A pointer to an array to be copied from the user.
 *
 *  @return INV_SUCCESS if successful; a non-zero error code otherwise.
 */
inv_error_t inv_set_accel_bias(long *data)
{
    INVENSENSE_FUNC_START;
    inv_error_t result = INV_SUCCESS;
    long biasTmp;
    int i, j;
    unsigned char regs[6];
    struct mldl_cfg *mldl_cfg = inv_get_dl_config();

    for (i = 0; i < ACCEL_NUM_AXES; i++) {
        inv_obj.accel_bias[i] = data[i];
        if (inv_obj.accel_sens != 0 && mldl_cfg && mldl_cfg->pdata) {
            long long tmp64;
            inv_obj.scaled_accel_bias[i] = 0;
            for (j = 0; j < ACCEL_NUM_AXES; j++) {
                inv_obj.scaled_accel_bias[i] +=
                    data[j] *
                    (long)mldl_cfg->pdata->accel.orientation[i * 3 + j];
            }
            tmp64 = (long long)inv_obj.scaled_accel_bias[i] << 13;
            biasTmp = (long)(tmp64 / inv_obj.accel_sens);
        } else {
            biasTmp = 0;
        }
        if (biasTmp < 0)
            biasTmp += 65536L;
        regs[2 * i + 0] = (unsigned char)(biasTmp / 256);
        regs[2 * i + 1] = (unsigned char)(biasTmp % 256);
    }
    result = inv_set_mpu_memory(KEY_D_1_8, 2, &regs[0]);
    if (result) {
        LOG_RESULT_LOCATION(result);
        return result;
    }
    result = inv_set_mpu_memory(KEY_D_1_10, 2, &regs[2]);
    if (result) {
        LOG_RESULT_LOCATION(result);
        return result;
    }
    result = inv_set_mpu_memory(KEY_D_1_2, 2, &regs[4]);
    if (result) {
        LOG_RESULT_LOCATION(result);
        return result;
    }

    return INV_SUCCESS;
}

/**
 *  @brief  inv_set_gyro_temp_slope is used to set the temperature
 *          compensation algorithm's estimate of the gyroscope bias temperature
 *          coefficient.
 *          The argument array elements are ordered X,Y,Z.
 *          Values are in units of dps per deg C (degrees per second per degree
 *          Celcius), and scaled such that 1 dps per deg C = 2^16 LSBs.
 *          Please refer to the provided "9-Axis Sensor Fusion
 *          Application Note" document.
 *
 *  @brief  inv_set_gyro_temp_slope is used to set Gyro temperature slope
 *
 *
 *  @pre    MLDmpOpen() \ifnot UMPL or
 *          MLDmpPedometerStandAloneOpen() \endif
 *
 *  @param  data        A pointer to an array to be copied from the user.
 *
 *  @return INV_SUCCESS if successful; a non-zero error code otherwise.
 */
inv_error_t inv_set_gyro_temp_slope(long *data)
{
    INVENSENSE_FUNC_START;
    inv_error_t result = INV_SUCCESS;
    int i;
    long sf;
    unsigned char regs[3];

    inv_obj.factory_temp_comp = 1;
    inv_obj.temp_slope[0] = data[0];
    inv_obj.temp_slope[1] = data[1];
    inv_obj.temp_slope[2] = data[2];
    for (i = 0; i < GYRO_NUM_AXES; i++) {
        sf = -inv_obj.temp_slope[i] / 1118;
        if (sf > 127) {
            sf -= 256;
        }
        regs[i] = (unsigned char)sf;
    }
    result = inv_set_offsetTC(regs);

    if (result) {
        LOG_RESULT_LOCATION(result);
        return result;
    }
    return INV_SUCCESS;
}

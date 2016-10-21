/**
 * mlcompat! where symbols referenced by libinvensense_mpl.so go to die. :(
 * They may one day rise again, e.g. inv_get_motion_state looked fun, but
 * this helps keep track of where things are used and by whom are they used.
 */
#include "mlcompat.h"
#include "ml.h"
#include "mlMathFunc.h"
#include "mlmath.h"

void inv_q_multf(const float *q1, const float *q2, float *qProd)
{
    qProd[0] = (q1[0] * q2[0] - q1[1] * q2[1] - q1[2] * q2[2] - q1[3] * q2[3]);
    qProd[1] = (q1[0] * q2[1] + q1[1] * q2[0] + q1[2] * q2[3] - q1[3] * q2[2]);
    qProd[2] = (q1[0] * q2[2] - q1[1] * q2[3] + q1[2] * q2[0] + q1[3] * q2[1]);
    qProd[3] = (q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1] + q1[3] * q2[0]);
}

void inv_q_addf(float *q1, float *q2, float *qSum)
{
    qSum[0] = q1[0] + q2[0];
    qSum[1] = q1[1] + q2[1];
    qSum[2] = q1[2] + q2[2];
    qSum[3] = q1[3] + q2[3];
}

void inv_q_normalizef(float *q)
{
    float normSF = 0;
    float xHalf = 0;
    normSF = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    if (normSF < 2) {
        xHalf = 0.5f * normSF;
        normSF = normSF * (1.5f - xHalf * normSF * normSF);
        normSF = normSF * (1.5f - xHalf * normSF * normSF);
        normSF = normSF * (1.5f - xHalf * normSF * normSF);
        normSF = normSF * (1.5f - xHalf * normSF * normSF);
        q[0] *= normSF;
        q[1] *= normSF;
        q[2] *= normSF;
        q[3] *= normSF;
    } else {
        q[0] = 1.0;
        q[1] = 0.0;
        q[2] = 0.0;
        q[3] = 0.0;
    }
    normSF = (q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
}

/** Performs a length 4 vector normalization with a square root.
* @param[in,out] vector to normalize. Returns [1,0,0,0] is magnitude is zero.
*/
void inv_q_norm4(float *q)
{
    float mag;
    mag = sqrtf(q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3]);
    if (mag) {
        q[0] /= mag;
        q[1] /= mag;
        q[2] /= mag;
        q[3] /= mag;
    } else {
        q[0] = 1.f;
        q[1] = 0.f;
        q[2] = 0.f;
        q[3] = 0.f;
    }
}

void inv_q_invertf(const float *q, float *qInverted)
{
    qInverted[0] = q[0];
    qInverted[1] = -q[1];
    qInverted[2] = -q[2];
    qInverted[3] = -q[3];
}

void inv_matrix_det_incd(double *a, double *b, int *n, int x, int y)
{
    int k, l, i, j;
    for (i = 0, k = 0; i < *n; i++, k++) {
        for (j = 0, l = 0; j < *n; j++, l++) {
            if (i == x)
                i++;
            if (j == y)
                j++;
            *(b + 10 * k + l) = *(a + 10 * i + j);
        }
    }
    *n = *n - 1;
}

double inv_matrix_detd(double *p, int *n)
{
    double d[10][10], sum = 0;
    int i, j, m;
    m = *n;
    if (*n == 2)
        return (*p ** (p + 11) - *(p + 1) ** (p + 10));
    for (i = 0, j = 0; j < m; j++) {
        *n = m;
        inv_matrix_det_incd(p, &d[0][0], n, i, j);
        sum =
            sum + *(p + 10 * i + j) * SIGNM(i + j) * inv_matrix_detd(&d[0][0],
                                                                     n);
    }

    return (sum);
}


__attribute__((noinline))
static inv_error_t log_call_error(const char *functionName)
{
    ALOGE("%s: Why are we being called?!", functionName);
    return INV_ERROR_INVALID_PARAMETER;
}

inv_error_t inv_pressure_supervisor()
{
    return log_call_error(__func__);
}

int inv_get_motion_state()
{
    log_call_error(__func__);
    return 0;
}

void inv_filter_long(struct filter_long *state __unused, long x __unused)
{
    log_call_error(__func__);
}

inv_error_t inv_get_array(int dataSet __unused, long *data __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_get_float_array(int dataSet __unused, float *data __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_get_mpu_memory_original(unsigned short key __unused, unsigned short length __unused, unsigned char *buffer __unused)
{
    return log_call_error(__func__);
}

unsigned short inv_dl_get_address(unsigned short key __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_set_external_sync(unsigned char extSync __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_get_quantized_accel(long *data __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_get_unquantized_accel(long *data __unused)
{
    return log_call_error(__func__);
}

int_fast16_t inv_get_sample_frequency()
{
    log_call_error(__func__);
    return 0;
}

inv_error_t inv_get_gyro_sensor(long *data __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_send_packet_number(uint_fast16_t accuracy __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_send_sensor_data(uint_fast16_t elements __unused, uint_fast16_t accuracy __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_send_cntrl_data(uint_fast16_t elements __unused, uint_fast16_t accuracy __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_send_quantized_accel(uint_fast16_t elements __unused, uint_fast16_t accuracy __unused)
{
    return log_call_error(__func__);
}

uint_fast16_t inv_get_fifo_packet_size()
{
    log_call_error(__func__);
    return 0;
}

inv_error_t inv_get_eis(long *data __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_get_temperature(long *data __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_get_control_data(long *controlSignal __unused, long *gridNum __unused, long *gridChange __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_set_gyro_bias_in_dps(const long *bias __unused, int mode __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_set_gyro_bias_in_dps_float(const float *bias __unused, int mode __unused)
{
    return log_call_error(__func__);
}

inv_error_t inv_get_linear_accel_in_world(long *data __unused)
{
    return log_call_error(__func__);
}

#ifndef __INV_COMPAT_H__
#define __INV_COMPAT_H__

#define LOG_TAG "mlcompat"

#include "mltypes.h"
#include <utils/Log.h>

#ifndef DEPRECATED
#define DEPRECATED __attribute__((deprecated))
#endif

#ifdef __cplusplus
extern "C" {
#endif

    struct filter_long {
        int length;
        const long *b;
        const long *a;
        long *x;
        long *y;
    };

    DEPRECATED void inv_q_multf(const float *q1, const float *q2, float *qProd);
    DEPRECATED void inv_q_addf(float *q1, float *q2, float *qSum);
    DEPRECATED void inv_q_normalizef(float *q);
    DEPRECATED void inv_q_norm4(float *q);
    DEPRECATED void inv_q_invertf(const float *q, float *qInverted);
    DEPRECATED void inv_matrix_det_incd(double *a, double *b, int *n, int x, int y);
    DEPRECATED double inv_matrix_detd(double *p, int *n);

    DEPRECATED inv_error_t inv_pressure_supervisor(void);
    DEPRECATED int inv_get_motion_state(void);
    DEPRECATED void inv_filter_long(struct filter_long *state, long x);
    DEPRECATED inv_error_t inv_get_array(int dataSet, long *data);
    DEPRECATED inv_error_t inv_get_float_array(int dataSet, float *data);
    DEPRECATED inv_error_t inv_get_mpu_memory_original(unsigned short key, unsigned short length, unsigned char *buffer);
    DEPRECATED unsigned short inv_dl_get_address(unsigned short key);
    DEPRECATED inv_error_t inv_set_external_sync(unsigned char extSync);
    DEPRECATED inv_error_t inv_get_quantized_accel(long *data);
    DEPRECATED inv_error_t inv_get_unquantized_accel(long *data);
    DEPRECATED int_fast16_t inv_get_sample_frequency(void);
    DEPRECATED inv_error_t inv_get_gyro_sensor(long *data);
    DEPRECATED inv_error_t inv_send_packet_number(uint_fast16_t accuracy);
    DEPRECATED inv_error_t inv_send_sensor_data(uint_fast16_t elements, uint_fast16_t accuracy);
    DEPRECATED inv_error_t inv_send_cntrl_data(uint_fast16_t elements, uint_fast16_t accuracy);
    DEPRECATED inv_error_t inv_send_quantized_accel(uint_fast16_t elements, uint_fast16_t accuracy);
    DEPRECATED uint_fast16_t inv_get_fifo_packet_size(void);
    DEPRECATED inv_error_t inv_get_eis(long *data);
    DEPRECATED inv_error_t inv_get_temperature(long *data);
    DEPRECATED inv_error_t inv_get_control_data(long *controlSignal, long *gridNum, long *gridChange);
    DEPRECATED inv_error_t inv_set_gyro_bias_in_dps(const long *bias, int mode);
    DEPRECATED inv_error_t inv_set_gyro_bias_in_dps_float(const float *bias, int mode);
    DEPRECATED inv_error_t inv_get_linear_accel_in_world(long *data);
#ifdef __cplusplus
}
#endif

#endif /* __INV_COMPAT_H__ */

#ifndef __INV_COMPAT_H__
#define __INV_COMPAT_H__

#define LOG_TAG "mlcompat"

#include "mltypes.h"
#include <utils/Log.h>

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

    inv_error_t inv_pressure_supervisor(void);

    int inv_get_motion_state(void);

    void inv_filter_long(struct filter_long *state, long x);
    void inv_q_multf(const float *q1, const float *q2, float *qProd);
    void inv_q_addf(float *q1, float *q2, float *qSum);
    void inv_q_normalizef(float *q);
    void inv_q_norm4(float *q);
    void inv_q_invertf(const float *q, float *qInverted);
    void inv_matrix_det_incd(double *a, double *b, int *n, int x, int y);
    double inv_matrix_detd(double *p, int *n);

    inv_error_t inv_get_array(int dataSet, long *data);
    inv_error_t inv_get_float_array(int dataSet, float *data);

    inv_error_t inv_get_mpu_memory_original(unsigned short key,
                                            unsigned short length,
                                            unsigned char *buffer);

    unsigned short inv_dl_get_address(unsigned short key);
    inv_error_t inv_set_external_sync(unsigned char extSync);
    inv_error_t inv_get_quantized_accel(long *data);
    inv_error_t inv_get_unquantized_accel(long *data);
    int_fast16_t inv_get_sample_frequency(void);
    inv_error_t inv_get_gyro_sensor(long *data);
    inv_error_t inv_send_sensor_data(uint_fast16_t elements,
                                     uint_fast16_t accuracy);
    inv_error_t inv_send_cntrl_data(uint_fast16_t elements,
                                    uint_fast16_t accuracy);
    uint_fast16_t inv_get_fifo_packet_size(void);
#ifdef __cplusplus
}
#endif

#endif /* __INV_COMPAT_H__ */
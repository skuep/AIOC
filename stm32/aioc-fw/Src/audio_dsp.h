#ifndef AUDIO_DSP_H_
#define AUDIO_DSP_H_

#include <stdint.h>

typedef struct {
    uint32_t sum;
    uint32_t integer_rate;
    uint32_t frac_rate_n, frac_rate_d;
    uint32_t current_sample, frac_samples_left;
    uint32_t output_samples;
    uint32_t output_buffer[128];
} rational_decimator_t;

void rational_decimator_init(rational_decimator_t* rd);
void rational_decimator_reset(rational_decimator_t* rd);
void rational_decimator_set_rate(rational_decimator_t* rd, uint32_t input_rate, uint32_t output_rate);
uint32_t rational_decimator_scale(rational_decimator_t* rd, uint32_t sample);
uint32_t rational_decimator_process_block_u16(rational_decimator_t* rd, uint16_t* buf, uint32_t len);
uint32_t* rational_decimator_get_outputs(rational_decimator_t* rd);

typedef struct {
	uint32_t current_val;
    uint32_t integer_rate;
    uint32_t frac_rate_n, frac_rate_d;
    uint32_t current_sample, frac_samples_left;
} rational_interpolator_t;

void rational_interpolator_init(rational_interpolator_t* ri);
void rational_interpolator_reset(rational_interpolator_t* ri);
void rational_interpolator_set_rate(rational_interpolator_t* ri, uint32_t input_rate, uint32_t output_rate);
void rational_interpolator_fill_buffer(rational_interpolator_t* ri, uint16_t* buf, uint32_t len);

#endif

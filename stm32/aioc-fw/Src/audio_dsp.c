#include "audio_dsp.h"

#include <string.h>

void rational_decimator_init(rational_decimator_t* rd) {
	memset(rd, 0, sizeof(*rd));
	rd->integer_rate = 1;
}

void rational_decimator_reset(rational_decimator_t* rd) {
	rd->sum = 0;
	rd->current_sample = 0;
	rd->output_samples = 0;
}

static uint32_t gcd(uint32_t a, uint32_t b) {
    while (b != 0) {
    	uint32_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

void rational_decimator_set_rate(rational_decimator_t* rd, uint32_t input_rate, uint32_t output_rate) {
	uint32_t n = input_rate % output_rate;
	uint32_t d = output_rate;
	rd->integer_rate = input_rate / output_rate;
	rd->frac_rate_n = n / gcd(n,d);
	rd->frac_rate_d = d / gcd(n,d);
}
uint32_t rational_decimator_get_integer_rate(rational_decimator_t* rd) {
	return rd->integer_rate;
}

uint32_t rational_decimator_process_block_u16(rational_decimator_t* rd, uint16_t* buf, uint32_t len) {
	uint32_t cur_sum = rd->sum;
	uint32_t cur_sample = rd->current_sample;
	uint32_t rate = rd->integer_rate;
	for(int i = 0;i < len;i++) {
		cur_sum += buf[i];
		cur_sample++;
		if (cur_sample >= rate) {
			rd->output_buffer[rd->output_samples++] = cur_sum;
			cur_sample = 0;
			cur_sum = 0;
		}
	}
	rd->sum = cur_sum;
	rd->current_sample = cur_sample;
	return rd->output_samples;
}

uint32_t* rational_decimator_get_outputs(rational_decimator_t* rd) {
	rd->output_samples = 0;
	return rd->output_buffer;
}

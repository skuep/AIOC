#include "audio_dsp.h"

#include <string.h>

void rational_decimator_init(rational_decimator_t* rd) {
    memset(rd, 0, sizeof(*rd));
    rd->integer_rate = 1;
}

void rational_decimator_reset(rational_decimator_t* rd) {
    rd->sum = 0;
    rd->current_sample = 0;
    rd->frac_samples_left = rd->frac_rate_n;
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

static uint32_t min(uint32_t a, uint32_t b) {
    return (a>b) ? b : a;
}

void rational_decimator_set_rate(rational_decimator_t* rd, uint32_t input_rate, uint32_t output_rate) {
    uint32_t n = input_rate % output_rate;
    uint32_t d = output_rate;
    rd->integer_rate = input_rate / output_rate;
    rd->frac_rate_n = n / gcd(n,d);
    rd->frac_rate_d = d / gcd(n,d);
}
uint32_t rational_decimator_scale(rational_decimator_t* rd, uint32_t sample) {
    uint64_t scaled_sample = sample;
    scaled_sample *= rd->frac_rate_d;
    scaled_sample /= rd->integer_rate * rd->frac_rate_d + rd->frac_rate_n;
    return scaled_sample;
}

uint32_t rational_decimator_process_block_u16(rational_decimator_t* rd, uint16_t* buf, uint32_t len) {
    uint32_t cur_sum = rd->sum;
    uint32_t cur_sample = rd->current_sample;
    uint32_t rate = rd->integer_rate;
    while(len > 0) {
        uint32_t next_block = min(min(rate - cur_sample, len), rate);
#pragma GCC unroll 4
        for(int i = 0;i < next_block;i++) {
            cur_sum += *buf++;
        }
        cur_sample += next_block;
        len -= next_block;

        if (cur_sample >= rate) {
            uint32_t frac_samples_left = rd->frac_samples_left;
            if (frac_samples_left == 0) {
                rd->output_buffer[rd->output_samples++] = cur_sum;
                cur_sample = 0;
                cur_sum = 0;
            } else if (len > 0) {
                // Handle the fractional sample if there is a sample left
                uint32_t cur_frac_sample = *buf++;
                len--;

                // Add the fractional sample
                uint32_t n = rd->frac_rate_n;
                uint32_t d = rd->frac_rate_d;
                cur_sum += cur_frac_sample * frac_samples_left / d;

                // Write the completed sample to output buffer
                rd->output_buffer[rd->output_samples++] = cur_sum;

                // Initialize the next block with the leftover fractional sample;
                uint32_t next_frac_left = (d - frac_samples_left);
                cur_sum = cur_frac_sample * next_frac_left / d;

                // Calculate the next fractional sample amount
                if (next_frac_left > n) {
                    cur_sample = 1;
                    rd->frac_samples_left = n + d - next_frac_left;
                } else {
                    cur_sample = 0;
                    rd->frac_samples_left = n - next_frac_left;
                }
            }
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

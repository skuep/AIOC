#include "io.h"
#include "fox_hunt.h"
#include "morse.h"

uint8_t identifying = 0;
uint8_t seconds_passed = 0;
uint8_t key_on = 0;
uint16_t cycles_remaining;
uint8_t timings_length;
uint8_t timings_index = 0;
uint8_t timings[MAX_TIMINGS];

/* 750 Hz sine wave at 48000 Hz sample rate and 16 bits/sample */
#define NS 64
uint16_t sin_wave[NS] = {
		32768, 36030, 39260, 42426, 45496, 48439, 51226, 53830, 56225, 58386,
		60293, 61926, 63270, 64310, 65037, 65443, 65525, 65281, 64713, 63829,
		62635, 61145, 59373, 57336, 55055, 52553, 49854, 46985, 43975, 40853,
		37651, 34401, 31134, 27884, 24682, 21560, 18550, 15681, 12982, 10480,
		8199, 6162, 4390, 2900, 1706, 822, 254, 10, 92, 498,
		1225, 2265, 3609, 5242, 7149, 9310, 11705, 14309, 17096, 20039,
		23109, 26275, 29505, 32767,
};
uint8_t sample_index = 0;

void fox_hunt_init(void) {
	char id[MAX_ID];

	/* Read the ID from the settings registers */
	id[0]  = settingsRegMap[SETTINGS_REG_FOX_ID0] >> 0;
	id[1]  = settingsRegMap[SETTINGS_REG_FOX_ID0] >> 8;
	id[2]  = settingsRegMap[SETTINGS_REG_FOX_ID0] >> 16;
	id[3]  = settingsRegMap[SETTINGS_REG_FOX_ID0] >> 24;
	id[4]  = settingsRegMap[SETTINGS_REG_FOX_ID1] >> 0;
	id[5]  = settingsRegMap[SETTINGS_REG_FOX_ID1] >> 8;
	id[6]  = settingsRegMap[SETTINGS_REG_FOX_ID1] >> 16;
	id[7]  = settingsRegMap[SETTINGS_REG_FOX_ID1] >> 24;
	id[8]  = settingsRegMap[SETTINGS_REG_FOX_ID2] >> 0;
	id[9]  = settingsRegMap[SETTINGS_REG_FOX_ID2] >> 8;
	id[10] = settingsRegMap[SETTINGS_REG_FOX_ID2] >> 16;
	id[11] = settingsRegMap[SETTINGS_REG_FOX_ID2] >> 24;
	id[12] = settingsRegMap[SETTINGS_REG_FOX_ID3] >> 0;
	id[13] = settingsRegMap[SETTINGS_REG_FOX_ID3] >> 8;
	id[14] = settingsRegMap[SETTINGS_REG_FOX_ID3] >> 16;
	id[15] = settingsRegMap[SETTINGS_REG_FOX_ID3] >> 24;

	set_timings(id, timings, &timings_length);
}

/* Returns non-zero if we actually pushed output to the DAC */
uint8_t fox_hunt_output(void) {
	/* Default to no audio output */
	uint16_t sample = 0;

	if (identifying) {
		/* If the key is down, pull samples from our sine waveform */
		if (key_on) {
			sample = sin_wave[sample_index++];
			if (sample_index == NS) {
				sample_index = 0;
			}
		}

		/* output the sample to the DAC (the TIM6 IRQ will trigger it) */
		DAC1->DHR12L1 = sample;

		/* state machine for morse output
		 * we ssume that we alternate between a sine wave and silence
		 * timings[] has the amount of time we spend for each element
		 */
		cycles_remaining--;
		if (cycles_remaining == 0) {
			timings_index++;
			if (timings_index == timings_length) {
				/* All done IDing */
				identifying = 0;
				IO_PTTDeassert(IO_PTT_MASK_PTT1);
			} else {
				/* Move on to the next timing */
				cycles_remaining = timings[timings_index] * DIT_CYCLES;
				/* if we were silent start making noise, if we were making noise be silent */
				key_on = key_on == 0 ? 1 : 0;
			}
		}
		return 1;
	}
	return 0;
}

void fox_hunt_task(void) {
	if ((settingsRegMap[SETTINGS_REG_FOX_INTERVAL] != 0) &&
		(seconds_passed++ > settingsRegMap[SETTINGS_REG_FOX_INTERVAL]) &&
		(identifying == 0)) {

		seconds_passed = 0;
		IO_PTTAssert(IO_PTT_MASK_PTT1);
		/* Make sure the TIM6 IRQ is enabled (it won't be if there's no USB audio) */
		NVIC_EnableIRQ(TIM6_DAC1_IRQn);
		identifying = 1;
		timings_index = 0;
		cycles_remaining = timings[timings_index] * DIT_CYCLES;
		key_on = 0;
	}
}

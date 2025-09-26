#include "io.h"
#include "fox_hunt.h"
#include "morse.h"

#define FOXHUNT_SAMPLEFREQ      48000

uint8_t identifying = 0;
uint8_t seconds_passed = 0;
uint8_t key_on = 0;
uint16_t cycles_remaining;
uint8_t timings_length;
uint8_t timings_index = 0;
uint8_t timings[FOXHUNT_MAX_TIMINGS];

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

static void Timer_DAC_Init(void)
{
    /* Calculate clock rate divider for requested sample rate with rounding */
    uint32_t timerFreq = (HAL_RCC_GetHCLKFreq() == HAL_RCC_GetPCLK2Freq()) ? HAL_RCC_GetPCLK2Freq() : 2 * HAL_RCC_GetPCLK2Freq();
    uint32_t rateDivider = (timerFreq + FOXHUNT_SAMPLEFREQ / 2) / FOXHUNT_SAMPLEFREQ;

    /* Enable clock and (re-) initialize timer */
    __HAL_RCC_TIM15_CLK_ENABLE();

    /* TIM15_TRGO triggers DAC */
    TIM15->CR1 &= ~TIM_CR1_CEN;
    TIM15->CR1 = TIM_CLOCKDIVISION_DIV1 | TIM_COUNTERMODE_UP | TIM_AUTORELOAD_PRELOAD_ENABLE;
    TIM15->CR2 = TIM_TRGO_UPDATE;
    TIM15->PSC = 0;
    TIM15->ARR = rateDivider - 1;
    TIM15->EGR = TIM_EGR_UG;

    TIM15->DIER = TIM_DIER_UIE;
    TIM15->CR1 |= TIM_CR1_CEN;

    NVIC_SetPriority(TIM15_IRQn, AIOC_IRQ_PRIO_AUDIO);
}

static void DAC_Init(void)
{
    __HAL_RCC_DAC1_CLK_ENABLE();

    /* Make sure DAC is disabled */
    DAC->CR = 0x00;

    /* Select TIM15 TRGO as trigger and enable DAC */
    DAC->CR = (0x3 << DAC_CR_TSEL1_Pos) | DAC_CR_TEN1 | DAC_CR_EN1;

    /* Output VDD/2 */
    DAC1->DHR12L1 = 32768;
}

void FoxHunt_Init(void) {
    /* TODO: Move this into a state machine into Tick. e.g. if !initialized and interval > 0 -> initialize */
	char id[FOXHUNT_MAX_ID];

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

	if (settingsRegMap[SETTINGS_REG_FOX_INTERVAL] != 0) {
        /* Set up the DAC and timer. Note, this needs to happen after the "regular" USB Audio Subsystem Init,
         * to overwrite their DAC configuration for our purpose here. */
        Timer_DAC_Init();
        DAC_Init();
	}
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

		/* output the sample to the DAC (the TIM15 IRQ will trigger it) */
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

void FoxHunt_Tick(void) {
	if ((settingsRegMap[SETTINGS_REG_FOX_INTERVAL] != 0) &&
		(seconds_passed++ > settingsRegMap[SETTINGS_REG_FOX_INTERVAL]) &&
		(identifying == 0)) {

		seconds_passed = 0;
		IO_PTTAssert(IO_PTT_MASK_PTT1);
		/* Make sure the TIM15 IRQ is enabled (it won't be if there's no USB audio) */
		NVIC_EnableIRQ(TIM15_IRQn);
		identifying = 1;
		timings_index = 0;
		cycles_remaining = timings[timings_index] * DIT_CYCLES;
		key_on = 0;
	}
}

void TIM15_IRQHandler(void)
{
    if (TIM15->SR & TIM_SR_UIF) {
        TIM15->SR = (uint32_t) ~TIM_SR_UIF;

        if (fox_hunt_output() != 0)
            return;


        int16_t sample = 0x0000;
#if 0
        /* Read from FIFO, leave sample at 0 if fifo empty */
        tud_audio_read(&sample, sizeof(sample));

        /* Automatic PTT */
        uint16_t pttThreshold = (settingsRegMap[SETTINGS_REG_VPTT_LVLCTRL] & SETTINGS_REG_VPTT_LVLCTRL_THRSHLD_MASK) >> SETTINGS_REG_VPTT_LVLCTRL_THRSHLD_OFFS;

        if (!speakerMute[1] && ( (sample > pttThreshold) || (sample < -pttThreshold) )) {
            /* Reset timeout and make sure timer is enabled */
            TIM16->EGR = TIM_EGR_UG; /* Generate an update event in the timer */
        }

        /* Get volume */
        uint16_t volume = !speakerMute[1] ? speakerLinVolume[1] : 0;

        /* Scale with 16-bit unsigned volume and round */
        sample = (int16_t) (((int32_t) sample * volume + (sample > 0 ? 32768 : -32768)) / 65536);
#endif
        /* Load DAC holding register with sample */
        DAC1->DHR12L1 = ((int32_t) sample + 32768) & 0xFFFFU;

    }
}

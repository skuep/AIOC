#include "io.h"
#include "fox_hunt.h"
#include "morse.h"

#define FOXHUNT_SAMPLERATE      48000
#define FOXHUNT_VOLUME          32768
#define FOXHUNT_CARRIERLUT_SIZE (sizeof(carrierLUT)/sizeof(*carrierLUT))
#define FOXHUNT_WPM             20

static uint8_t isIdentifying = 0;
static uint8_t secondsPassed = 0;
static uint8_t isKeying = 0;
static uint16_t remainingCycles;
static uint8_t timingsLength;
static uint8_t timingsIndex = 0;
static uint8_t timingsLUT[FOXHUNT_MAX_TIMINGS];
static uint8_t sampleIndex = 0;

/* 750 Hz sine wave at 48000 Hz sample rate and 16 bits/sample */
static int16_t carrierLUT[] = {
        0,  3252,  6482,  9658, 12728, 15671, 18458, 21062, 23457, 25618,
    27525, 29158, 30502, 31542, 32269, 32675, 32757, 32513, 31945, 31061,
    29867, 28377, 26605, 24568, 22287, 19785, 17086, 14217, 11207,  8085,
     4883,  2173,   -434, -3884, -6886, -10108, -13118, -15987, -18686, -21188,
   -24569, -26606, -28378, -29868, -31062, -31946, -32514, -32758, -32676, -32270,
   -31543, -30503, -29159, -27526, -25619, -23458, -21063, -18459, -15672, -12729,
    -9659, -6483, -3253,     -1
};

static void Timer_DAC_Init(void)
{
    /* Calculate clock rate divider for requested sample rate with rounding */
    uint32_t timerFreq = (HAL_RCC_GetHCLKFreq() == HAL_RCC_GetPCLK2Freq()) ? HAL_RCC_GetPCLK2Freq() : 2 * HAL_RCC_GetPCLK2Freq();
    uint32_t rateDivider = (timerFreq + FOXHUNT_SAMPLERATE / 2) / FOXHUNT_SAMPLERATE;

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
	char messageBuffer[FOXHUNT_MAX_CHARS];

	/* Read the ID from the settings registers */
	messageBuffer[0]  = settingsRegMap[SETTINGS_REG_FOX_ID0] >> 0;
	messageBuffer[1]  = settingsRegMap[SETTINGS_REG_FOX_ID0] >> 8;
	messageBuffer[2]  = settingsRegMap[SETTINGS_REG_FOX_ID0] >> 16;
	messageBuffer[3]  = settingsRegMap[SETTINGS_REG_FOX_ID0] >> 24;
	messageBuffer[4]  = settingsRegMap[SETTINGS_REG_FOX_ID1] >> 0;
	messageBuffer[5]  = settingsRegMap[SETTINGS_REG_FOX_ID1] >> 8;
	messageBuffer[6]  = settingsRegMap[SETTINGS_REG_FOX_ID1] >> 16;
	messageBuffer[7]  = settingsRegMap[SETTINGS_REG_FOX_ID1] >> 24;
	messageBuffer[8]  = settingsRegMap[SETTINGS_REG_FOX_ID2] >> 0;
	messageBuffer[9]  = settingsRegMap[SETTINGS_REG_FOX_ID2] >> 8;
	messageBuffer[10] = settingsRegMap[SETTINGS_REG_FOX_ID2] >> 16;
	messageBuffer[11] = settingsRegMap[SETTINGS_REG_FOX_ID2] >> 24;
	messageBuffer[12] = settingsRegMap[SETTINGS_REG_FOX_ID3] >> 0;
	messageBuffer[13] = settingsRegMap[SETTINGS_REG_FOX_ID3] >> 8;
	messageBuffer[14] = settingsRegMap[SETTINGS_REG_FOX_ID3] >> 16;
	messageBuffer[15] = settingsRegMap[SETTINGS_REG_FOX_ID3] >> 24;

	timingsLength = Morse_GenerateTimings(messageBuffer, FOXHUNT_MAX_CHARS, timingsLUT, FOXHUNT_MAX_TIMINGS);

	if (settingsRegMap[SETTINGS_REG_FOX_INTERVAL] != 0) {
        /* Set up the DAC and timer. Note, this needs to happen after the "regular" USB Audio Subsystem Init,
         * to overwrite their DAC configuration for our purpose here. */
        Timer_DAC_Init();
        DAC_Init();

        /* Make sure the TIM15 IRQ is enabled */
        NVIC_EnableIRQ(TIM15_IRQn);
	}
}

void FoxHunt_Tick(void) {
	if ((settingsRegMap[SETTINGS_REG_FOX_INTERVAL] != 0) &&
		(secondsPassed++ > settingsRegMap[SETTINGS_REG_FOX_INTERVAL]) &&
		(isIdentifying == 0)) {

		secondsPassed = 0;
		IO_PTTAssert(IO_PTT_MASK_PTT1);

		isIdentifying = 1;
		timingsIndex = 0;
		remainingCycles = timingsLUT[timingsIndex] * ((uint32_t) (MORSE_UNIT_LENGTH * FOXHUNT_SAMPLERATE) / FOXHUNT_WPM);
		isKeying = 0;
	}
}

void TIM15_IRQHandler(void)
{
    if (TIM15->SR & TIM_SR_UIF) {
        TIM15->SR = (uint32_t) ~TIM_SR_UIF;

        int16_t sample = 0x0000;

        if (isIdentifying) {
            /* If the key is down, pull samples from our sine waveform */
            if (isKeying) {
                sample = carrierLUT[sampleIndex++];
                if (sampleIndex == FOXHUNT_CARRIERLUT_SIZE) {
                    sampleIndex = 0;
                }
            }

            /* state machine for morse output
             * we ssume that we alternate between a sine wave and silence
             * timings[] has the amount of time we spend for each element
             */
            remainingCycles--;
            if (remainingCycles == 0) {
                timingsIndex++;
                if (timingsIndex == timingsLength) {
                    /* All done IDing */
                    isIdentifying = 0;
                    IO_PTTDeassert(IO_PTT_MASK_PTT1);
                } else {
                    /* Move on to the next timing */
                    remainingCycles = timingsLUT[timingsIndex] * ((uint32_t) (MORSE_UNIT_LENGTH * FOXHUNT_SAMPLERATE) / FOXHUNT_WPM);
                    /* if we were silent start making noise, if we were making noise be silent */
                    isKeying = isKeying == 0 ? 1 : 0;
                }
            }
        }

        /* Get volume */
        uint16_t volume = FOXHUNT_VOLUME;

        /* Scale with 16-bit unsigned volume and round */
        sample = (int16_t) (((int32_t) sample * volume + (sample > 0 ? 32768 : -32768)) / 65536);

        /* Load DAC holding register with sample */
        DAC1->DHR12L1 = ((int32_t) sample + 32768) & 0xFFFFU;

    }
}

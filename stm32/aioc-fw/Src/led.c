#include "led.h"
#include "stm32f3xx_hal.h"
#include <assert.h>

uint8_t LedIdleLevels[2] = {LED_IDLE_LEVEL, LED_OFF_LEVEL};
uint8_t LedStates[2] = {0, 0};
uint8_t LedModes[2] = {LED_MODE_SOLID, LED_MODE_SOLID};
uint8_t LedLevels[2] = {LED_IDLE_LEVEL, LED_IDLE_LEVEL};
uint16_t LedCounter = 0;
uint16_t LedCounterPrev = 0;

#define LED_SET1(x) LED_TIMER->CCR3 = (512 - ( ((uint32_t) (x)) & 0xFF ))
#define LED_SET2(x) LED_TIMER->CCR4 = ( ((uint32_t) (x)) & 0xFF )

void LED_TIMER_IRQ(void)
{
    LED_TIMER->SR = (uint32_t) ~TIM_SR_UIF;

    for (uint8_t i=0; i<2; i++) {
        uint8_t mode = LedModes[i];

        switch (mode) {
        case LED_MODE_SLOWPULSE4X:
        case LED_MODE_SLOWPULSE3X:
        case LED_MODE_SLOWPULSE2X:
        case LED_MODE_SLOWPULSE:
            if ( !(LedCounterPrev & 0x200) && (LedCounter & 0x200) ) {
                /* Rising Edge  */
                LedLevels[i] = LED_FULL_LEVEL;
            } else if ( (LedCounterPrev & 0x200) && !(LedCounter & 0x200) ) {
                /* Falling Edge */
                if (LedLevels[i] != LedIdleLevels[i]) {
                    /* Only reset colors and mode when a pulse has been carried out */
                    LedLevels[i] = LedIdleLevels[i];
                    LedModes[i] = mode > LED_MODE_SLOWPULSE ? mode - 1 : LED_MODE_SOLID;
                }
            }
            break;

        case LED_MODE_FASTPULSE4X:
        case LED_MODE_FASTPULSE3X:
        case LED_MODE_FASTPULSE2X:
        case LED_MODE_FASTPULSE:
            if ( !(LedCounterPrev & 0x40) && (LedCounter & 0x40) ) {
                /* Rising Edge  */
                LedLevels[i] = LED_FULL_LEVEL;
            } else if ( (LedCounterPrev & 0x40) && !(LedCounter & 0x40) ) {
                /* Falling Edge */
                if (LedLevels[i] != LedIdleLevels[i]) {
                    LedLevels[i] = LedIdleLevels[i];
                    LedModes[i] = mode > LED_MODE_FASTPULSE ? mode - 1 : LED_MODE_SOLID;
                }
            }
            break;

        case LED_MODE_SOLID:
            LedLevels[i] = LedStates[i] ? LED_FULL_LEVEL : LedIdleLevels[i];
            break;

        default:
            assert(0);
            break;
        }
    }

    /* LEDs are connected anti-parallel */
    LED_TIMER->CCR3 = (512 - (uint32_t) LedLevels[0]);
    LED_TIMER->CCR4 = ( (uint32_t) LedLevels[1]);

    /* Advance counters */
    LedCounterPrev = LedCounter;
    LedCounter = (LedCounter + 1) & 0xFFF;
}

void LED_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure LED pins as TIM4 CH3/CH4 */
    GPIO_InitTypeDef GpioLedInit = {
        .Pin = LED_GPIO_PIN1 | LED_GPIO_PIN2,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF2_TIM4
    };

    HAL_GPIO_Init(LED_GPIO, &GpioLedInit);

    /* Initialize Timer */
    __HAL_RCC_TIM4_CLK_ENABLE();

    LED_TIMER->CR1 = TIM_CLOCKDIVISION_DIV1 | TIM_COUNTERMODE_UP | TIM_AUTORELOAD_PRELOAD_ENABLE;
    LED_TIMER->ARR = 511;
    LED_TIMER->PSC = (2 * HAL_RCC_GetPCLK1Freq() / 500000) - 1;
    LED_TIMER->CCMR2 =  TIM_OCMODE_PWM1 << 8 | TIM_OCMODE_PWM1 | TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;
    LED_TIMER->CCER = (0 << TIM_CCER_CC3P_Pos) | (1 << TIM_CCER_CC4P_Pos) | TIM_CCER_CC3E | TIM_CCER_CC4E;
    LED_TIMER->CCR3 = 0;
    LED_TIMER->CCR4 = 512;
    LED_TIMER->EGR = TIM_EGR_UG;
    LED_TIMER->DIER = TIM_DIER_UIE;
    LED_TIMER->CR1 |= TIM_CR1_CEN;


    NVIC_EnableIRQ(TIM4_IRQn);
}

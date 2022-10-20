#include "led.h"
#include "stm32f3xx_hal.h"

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
    LED_TIMER->PSC = (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_TIM1) / 500000) - 1;
    LED_TIMER->CCMR2 =  TIM_OCMODE_PWM1 << 8 | TIM_OCMODE_PWM1 | TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;
    LED_TIMER->CCER = (0 << TIM_CCER_CC3P_Pos) | (1 << TIM_CCER_CC4P_Pos) | TIM_CCER_CC3E | TIM_CCER_CC4E;
    LED_TIMER->CCR3 = 0;
    LED_TIMER->CCR4 = 512;
    LED_TIMER->EGR = TIM_EGR_UG;
    LED_TIMER->CR1 |= TIM_CR1_CEN;
}

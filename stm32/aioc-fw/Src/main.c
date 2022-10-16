#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"
#include "stm32f3xx_hal_rcc.h"
#include "stm32f3xx_hal_rcc_ex.h"
#include "stm32f3xx_hal_gpio.h"
#include "stm32f3xx_hal_tim.h"

static void SystemClock_Config(void)
{
    /* Enable external oscillator and configure PLL: 8 MHz (HSE) / 1 * 9 = 72 MHz */
    RCC_OscInitTypeDef OscConfig = {
        .OscillatorType = RCC_OSCILLATORTYPE_HSE,
        .HSEState = RCC_HSE_ON,
        .HSEPredivValue = RCC_HSE_PREDIV_DIV1,
        .LSEState = RCC_LSE_OFF,
        .HSIState = RCC_HSI_OFF,
        .HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
        .LSIState = RCC_LSI_ON,
        .PLL = {
            .PLLState = RCC_PLL_ON,
            .PLLSource = RCC_CFGR_PLLSRC_HSE_PREDIV,
            .PLLMUL = RCC_PLL_MUL9
        }
    };

    HAL_RCC_OscConfig(&OscConfig);

    /* Set correct peripheral clocks. 72 MHz (PLL) / 1.5 = 48 MHz */
    RCC_PeriphCLKInitTypeDef PeriphClk = {
        .PeriphClockSelection = RCC_PERIPHCLK_USB,
        .USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5
    };

    HAL_RCCEx_PeriphCLKConfig(&PeriphClk);

    /* Set up divider for maximum speeds and switch clock */
    RCC_ClkInitTypeDef ClkConfig = {
        .ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
        .SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
        .AHBCLKDivider = RCC_SYSCLK_DIV1,
        .APB1CLKDivider = RCC_HCLK_DIV2,
        .APB2CLKDivider = RCC_HCLK_DIV1
    };

    HAL_RCC_ClockConfig(&ClkConfig, FLASH_LATENCY_2);

    /* Enable MCO Pin to PLL/2 output */
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef GpioInit = {
        .Pin = GPIO_PIN_8,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_HIGH,
        .Alternate = GPIO_AF0_MCO
    };

    HAL_GPIO_Init(GPIOA, &GpioInit);
    HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_PLLCLK_DIV2, RCC_MCODIV_1);
}

#define LED1_SET(x) TIM4->CCR3 = ( ((uint32_t) ((x) * 255)) & 0xFF )
#define LED2_SET(x) TIM4->CCR4 = (511 - ( ((uint32_t) ((x) * 255)) & 0xFF ))

static void LED_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Configure LED pins as TIM4 CH3/CH4 */
    GPIO_InitTypeDef GpioLedInit = {
        .Pin = GPIO_PIN_8 | GPIO_PIN_9,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = GPIO_AF2_TIM4
    };

    HAL_GPIO_Init(GPIOB, &GpioLedInit);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);

    /* Initialize Timer */
	__HAL_RCC_TIM4_CLK_ENABLE();

	TIM4->CR1 = TIM_CLOCKDIVISION_DIV1 | TIM_COUNTERMODE_UP | TIM_AUTORELOAD_PRELOAD_ENABLE;
	TIM4->ARR = 511;
	TIM4->PSC = (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_TIM1) / 500000) - 1;
	TIM4->CCMR2 =  TIM_OCMODE_PWM1 << 8 | TIM_OCMODE_PWM1 | TIM_CCMR2_OC3PE | TIM_CCMR2_OC4PE;
	TIM4->CCER = (0 << TIM_CCER_CC3P_Pos) | (1 << TIM_CCER_CC4P_Pos) | TIM_CCER_CC3E | TIM_CCER_CC4E;
	TIM4->CCR3 = 0;
	TIM4->CCR4 = 511;
	TIM4->EGR = TIM_EGR_UG;
	TIM4->CR1 |= TIM_CR1_CEN;
}



int main(void)
{
    HAL_Init();
    SystemClock_Config();

    LED_Init();

    __HAL_RCC_SYSCFG_CLK_ENABLE();

    LED1_SET(1.0);
    LED2_SET(1.0);

    volatile uint32_t ii = 0;

    /* TODO - Add your application code here */
    while (1) {
        ii++;

    }

  return 0;
}

void NMI_Handler(void) {
}

void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

void MemManage_Handler(void) {
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) {
    }
}

void BusFault_Handler(void) {
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) {
    }
}

void UsageFault_Handler(void) {
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) {
    }
}

void SVC_Handler(void) {
}

void DebugMon_Handler(void) {
}

void PendSV_Handler(void) {
}

void SysTick_Handler(void) {
    HAL_IncTick();
}

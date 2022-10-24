#include "stm32f3xx.h"
#include "stm32f3xx_hal.h"
#include "led.h"
#include "usb.h"

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

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    __HAL_RCC_SYSCFG_CLK_ENABLE();

    LED_Init();
    LED_MODE(0, LED_MODE_SLOWPULSE2X);
    LED_MODE(1, LED_MODE_SLOWPULSE2X);

    USB_Init();

    while (1) {
        USB_Task();
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


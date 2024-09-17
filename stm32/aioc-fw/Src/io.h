#ifndef IO_H_
#define IO_H_

#include <stdint.h>
#include "stm32f3xx_hal.h"
#include "aioc.h"
#include "led.h"
#include "settings.h"

#define IO_PTT_MASK_NONE        0x00
#define IO_PTT_MASK_PTT1        0x01
#define IO_PTT_MASK_PTT2        0x02

#define IO_OUT_GPIO             GPIOA
#define IO_OUT_PIN_1            GPIO_PIN_1
#define IO_OUT_PIN_2            GPIO_PIN_0
#define IO_IN_GPIO              GPIOB
#define IO_IN_PIN_1             GPIO_PIN_6
#define IO_IN_PIN_2             GPIO_PIN_7
#define IO_IN_EXTI_ISR          EXTI9_5_IRQHandler
#define IO_IN_PIN_1_EXTI_PR     EXTI_PR_PR6
#define IO_IN_PIN_2_EXTI_PR     EXTI_PR_PR7
#define IO_IN_IRQN              EXTI9_5_IRQn

static inline void IO_PTTAssert(uint8_t pttMask)
{
    __disable_irq();

    if (pttMask & IO_PTT_MASK_PTT1) {
        IO_OUT_GPIO->BSRR = IO_OUT_PIN_1;
        LED_SET(1, 1);

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] |= SETTINGS_REG_INFO_AIOC0_PTT1STATE_MASK;
    }

    if (pttMask & IO_PTT_MASK_PTT2) {
        IO_OUT_GPIO->BSRR = IO_OUT_PIN_2;
        LED_SET(0, 1);

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] |= SETTINGS_REG_INFO_AIOC0_PTT2STATE_MASK;
    }

    __enable_irq();
}

static inline void IO_PTTDeassert(uint8_t pttMask)
{
    __disable_irq();

    if (pttMask & IO_PTT_MASK_PTT1) {
        IO_OUT_GPIO->BRR = IO_OUT_PIN_1;
        LED_SET(1, 0);

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] &= ~SETTINGS_REG_INFO_AIOC0_PTT1STATE_MASK;
    }

    if (pttMask & IO_PTT_MASK_PTT2) {
        IO_OUT_GPIO->BRR = IO_OUT_PIN_2;
        LED_SET(0, 0);

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] &= ~SETTINGS_REG_INFO_AIOC0_PTT2STATE_MASK;
    }

    __enable_irq();
}

static inline void IO_PTTControl(uint8_t pttMask)
{
    /* TODO: Using this function, both PTTs can only be asserted/deasserted simultaneously.
     * Switch to using fully event(edge) driven PTT assertion/deassertion using functions below */

    if (pttMask & IO_PTT_MASK_PTT1) {
        IO_PTTAssert(IO_PTT_MASK_PTT1);
    } else {
        IO_PTTDeassert(IO_PTT_MASK_PTT1);
    }

    if (pttMask & IO_PTT_MASK_PTT2) {
        IO_PTTAssert(IO_PTT_MASK_PTT2);
    } else {
        IO_PTTDeassert(IO_PTT_MASK_PTT2);
    }

}

static inline  uint8_t IO_PTTStatus(void)
{
    uint32_t inputReg = IO_OUT_GPIO->IDR;

    return (inputReg & IO_OUT_PIN_1 ? IO_PTT_MASK_PTT1 : 0) |
           (inputReg & IO_OUT_PIN_2 ? IO_PTT_MASK_PTT2 : 0);
}

static inline  void IO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Set up output GPIOs */
    GPIO_InitTypeDef outGPIO = {
        .Pin = (IO_OUT_PIN_2 | IO_OUT_PIN_1),
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = 0
    };
    HAL_GPIO_Init(IO_OUT_GPIO, &outGPIO);

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* Set up input GPIOs */
    GPIO_InitTypeDef inGPIO = {
        .Pin = (IO_IN_PIN_2 | IO_IN_PIN_1),
        .Mode = GPIO_MODE_IT_RISING_FALLING,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = 0
    };
    HAL_GPIO_Init(IO_IN_GPIO, &inGPIO);

    /* Clear any external interrupt and enable */
    EXTI->PR = IO_IN_PIN_1_EXTI_PR | IO_IN_PIN_2_EXTI_PR;
    NVIC_SetPriority(IO_IN_IRQN, AIOC_IRQ_PRIO_IO);
    NVIC_ClearPendingIRQ(IO_IN_IRQN);
    NVIC_EnableIRQ(IO_IN_IRQN);
}


#endif /* IO_H_ */

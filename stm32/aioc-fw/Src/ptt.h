#ifndef PTT_H_
#define PTT_H_

#include <stdint.h>
#include "stm32f3xx_hal.h"
#include "aioc.h"
#include "led.h"

#define PTT_MASK_NONE       0x00
#define PTT_MASK_PTT1       0x01
#define PTT_MASK_PTT2       0x02

#define PTT_GPIO            GPIOA
#define PTT_PIN_PTT1        GPIO_PIN_1
#define PTT_PIN_PTT2        GPIO_PIN_0

static inline void PTT_Control(uint8_t pttMask)
{
    __disable_irq();

    if (pttMask & PTT_MASK_PTT1) {
        PTT_GPIO->BSRR = PTT_PIN_PTT1;
        LED_SET(1, 1);
    } else {
        PTT_GPIO->BRR = PTT_PIN_PTT1;
        LED_SET(1, 0);
    }

    if (pttMask & PTT_MASK_PTT2) {
        PTT_GPIO->BSRR = PTT_PIN_PTT2;
        LED_SET(0, 1);
    } else {
        PTT_GPIO->BRR = PTT_PIN_PTT2;
        LED_SET(0, 0);
    }

    __enable_irq();
}

static inline  uint8_t PTT_Status(void)
{
    uint32_t inputReg = PTT_GPIO->IDR;

    return (inputReg & PTT_PIN_PTT1 ? PTT_MASK_PTT1 : 0) |
           (inputReg & PTT_PIN_PTT2 ? PTT_MASK_PTT2 : 0);
}

static inline  void PTT_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Set up PTT GPIOs */
    GPIO_InitTypeDef pttGPIO = {
        .Pin = (PTT_PIN_PTT2 | PTT_PIN_PTT1),
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = 0
    };
    HAL_GPIO_Init(PTT_GPIO, &pttGPIO);
}

#endif /* PTT_H_ */

#ifndef LED_H_
#define LED_H_

#include "stm32f3xx_hal.h"

#define LED_TIMER       TIM4
#define LED_GPIO        GPIOB
#define LED_GPIO_PIN1   GPIO_PIN_8
#define LED_GPIO_PIN2   GPIO_PIN_9

#define LED_IDLE_LEVEL  0.1
#define LED_FULL_LEVEL  1.0

#define LED_SET1(x) LED_TIMER->CCR3 = (512 - ( ((uint32_t) ((x) * 255)) & 0xFF ))
#define LED_SET2(x) LED_TIMER->CCR4 = ( ((uint32_t) ((x) * 255)) & 0xFF )

void LED_Init(void);

#endif /* LED_H_ */

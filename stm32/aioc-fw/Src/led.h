#ifndef LED_H_
#define LED_H_

#include "stm32f3xx_hal.h"

#define LED_TIMER       TIM4
#define LED_TIMER_IRQ   TIM4_IRQHandler
#define LED_GPIO        GPIOB
#define LED_GPIO_PIN1   GPIO_PIN_8
#define LED_GPIO_PIN2   GPIO_PIN_9

#define LED_OFF_LEVEL   0
#define LED_IDLE_LEVEL  25
#define LED_FULL_LEVEL  255

typedef enum {
    LED_MODE_SOLID = 0,
    LED_MODE_SLOWPULSE = 4,
    LED_MODE_SLOWPULSE2X = 5,
    LED_MODE_SLOWPULSE3X = 6,
    LED_MODE_SLOWPULSE4X = 7,
    LED_MODE_FASTPULSE = 8,
    LED_MODE_FASTPULSE2X = 9,
    LED_MODE_FASTPULSE3X = 10,
    LED_MODE_FASTPULSE4X = 11
} Led_Mode;

extern uint8_t LedStates[2];
extern uint8_t LedModes[2];

#define LED_SET(i, onOff) { __disable_irq(); LedStates[i] = !!(onOff); __enable_irq(); }
#define LED_MODE(i, mode) { __disable_irq(); LedModes[i] = (mode); __enable_irq(); }

void LED_Init(void);

#endif /* LED_H_ */

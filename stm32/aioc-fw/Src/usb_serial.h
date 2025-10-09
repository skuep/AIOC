#ifndef USB_SERIAL_H_
#define USB_SERIAL_H_

#include <stdint.h>
#include <stdbool.h>

#define USB_SERIAL_UART             USART1
#define USB_SERIAL_UART_IRQ         USART1_IRQHandler
#define USB_SERIAL_UART_PERIPHCLK   RCC_PERIPHCLK_USART1
#define USB_SERIAL_UART_RXTIMEOUT   20 /* in number of bits */
#define USB_SERIAL_UART_DEFBAUD     9600
#define USB_SERIAL_UART_GPIO        GPIOA
#define USB_SERIAL_UART_PIN_TX      GPIO_PIN_9
#define USB_SERIAL_UART_PIN_RX      GPIO_PIN_10

#define USB_SERIAL_LINESTATE_DCD    0x01
#define USB_SERIAL_LINESTATE_DSR    0x02
#define USB_SERIAL_LINESTATE_BREAK  0x04
#define USB_SERIAL_LINESTATE_RI     0x08

void USB_SerialInit(void);
void USB_SerialTask(void);

bool USB_SerialSendLineState(uint8_t lineState);

#endif /* USB_SERIAL_H_ */

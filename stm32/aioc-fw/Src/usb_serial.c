#include "usb_serial.h"
#include "stm32f3xx_hal.h"
#include "aioc.h"
#include "tusb.h"
#include "led.h"
#include "usb_descriptors.h"

void USB_SERIAL_UART_IRQ(void)
{
    uint32_t ISR = USB_SERIAL_UART->ISR;

    if (ISR & USART_ISR_TXE) {
        /* TX register is empty, load up another character */
        __disable_irq();
        uint32_t available = tud_cdc_n_available(ITF_NUM_CDC_0);
        if (available == 0) {
            /* No char left in fifo. Disable transmitter and TX-empty interrupt */
            USB_SERIAL_UART->CR1 &= (uint32_t) ~(USART_CR1_TE | USART_CR1_TXEIE);
        }
        __enable_irq();

        if (available > 0) {
             /* Write char from fifo */
            int32_t c = tud_cdc_n_read_char(ITF_NUM_CDC_0);
            TU_ASSERT(c != -1, /**/);
            USB_SERIAL_UART->TDR = (uint8_t) c;
            LED_MODE(1, LED_MODE_FASTPULSE);
        }
    }

    if (ISR & USART_ISR_RXNE) {
        /* RX register is not empty, get character and put into USB send buffer */
        if (tud_cdc_n_write_available(ITF_NUM_CDC_0) > 0) {
            uint8_t c = USB_SERIAL_UART->RDR;
            if ( !(USB_SERIAL_UART_GPIO->IDR & (USB_SERIAL_UART_PIN_PTT1 | USB_SERIAL_UART_PIN_PTT2)) ) {
                /* Only store character when no PTT is asserted (shares the same pin) */
                tud_cdc_n_write(ITF_NUM_CDC_0, &c, 1);
                LED_MODE(0, LED_MODE_FASTPULSE);
            }
        } else {
            /* No space in fifo currently. Pause this interrupt and re-enable later */
            __disable_irq();
            USB_SERIAL_UART->CR1 &= (uint32_t) ~USART_CR1_RXNEIE;
            __enable_irq();
        }
    }

    if (ISR & USART_ISR_RTOF) {
        USB_SERIAL_UART->ICR = USART_ICR_RTOCF;
        /* Receiver timeout. Flush data via USB. */
        tud_cdc_n_write_flush(ITF_NUM_CDC_0);
    }

    if (ISR & USART_ISR_ORE) {
        /* Overflow error */
        USB_SERIAL_UART->ICR = USART_ICR_ORECF;
        TU_ASSERT(0, /**/);
    }

    if (ISR & USART_ISR_FE) {
        USB_SERIAL_UART->ICR = USART_ISR_FE;
    }

    if (ISR & USART_ISR_NE) {
        USB_SERIAL_UART->ICR = USART_ISR_NE;
    }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
    if (itf == ITF_NUM_CDC_0) {
        /* This enables the transmitter and the TX-empty interrupt, which handles writing UART data */
        __disable_irq();
        USB_SERIAL_UART->CR1 |= USART_CR1_TE | USART_CR1_TXEIE;
        __enable_irq();
    }
}

// Invoked when space becomes available in TX buffer
void tud_cdc_tx_complete_cb(uint8_t itf)
{
    if (itf == ITF_NUM_CDC_0) {
        /* Re-enable UART RX-nonempty interrupt to handle reading UART data */
        __disable_irq();
        USB_SERIAL_UART->CR1 |= USART_CR1_RXNEIE;
        __enable_irq();
    }
}

// Invoked when line coding is change via SET_LINE_CODING
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
{
    if (itf == ITF_NUM_CDC_0) {
        /* Disable IRQs and UART */
        __disable_irq();
        USB_SERIAL_UART->CR1 &= (uint32_t) ~USART_CR1_UE;

        /* Calculate new baudrate */
        USB_SERIAL_UART->BRR = (HAL_RCCEx_GetPeriphCLKFreq(USB_SERIAL_UART_PERIPHCLK) + p_line_coding->bit_rate/2) / p_line_coding->bit_rate;

        if (p_line_coding->data_bits == 8) {
        } else {
            /* Support only 8 bit character size */
            TU_ASSERT(0, /**/);
        }

        if (p_line_coding->parity == 0) {
            /* No parity */
            USB_SERIAL_UART->CR1 = (USB_SERIAL_UART->CR1 & (uint32_t) ~(USART_CR1_PCE | USART_CR1_PS | USART_CR1_M | USART_CR1_M0))
                    | UART_PARITY_NONE;
        } else if (p_line_coding->parity == 1) {
            /* Odd parity */
            USB_SERIAL_UART->CR1 = (USB_SERIAL_UART->CR1 & (uint32_t) ~(USART_CR1_PCE | USART_CR1_PS | USART_CR1_M | USART_CR1_M0))
                    | UART_PARITY_ODD | UART_WORDLENGTH_9B;
        } else if (p_line_coding->parity == 2) {
            /* Even parity */
            USB_SERIAL_UART->CR1 = (USB_SERIAL_UART->CR1 & (uint32_t) ~(USART_CR1_PCE | USART_CR1_PS | USART_CR1_M | USART_CR1_M0))
                    | UART_PARITY_EVEN | UART_WORDLENGTH_9B;
        } else {
            /* Other parity modes are not supported */
            TU_ASSERT(0, /**/);
        }

        if (p_line_coding->stop_bits == 0) {
            /* 1 stop bit */
            USB_SERIAL_UART->CR2 = (USB_SERIAL_UART->CR2 & (uint32_t) ~USART_CR2_STOP) | UART_STOPBITS_1;
        } else if (p_line_coding->stop_bits == 1) {
            /* 1.5 stop bit */
            USB_SERIAL_UART->CR2 = (USB_SERIAL_UART->CR2 & (uint32_t) ~USART_CR2_STOP) | UART_STOPBITS_1_5;
        } else if (p_line_coding->stop_bits == 2) {
            /* 2 stop bit */
            USB_SERIAL_UART->CR2 = (USB_SERIAL_UART->CR2 & (uint32_t) ~USART_CR2_STOP) | UART_STOPBITS_2;
        } else {
            /* Other stop bits unsupported */
            TU_ASSERT(0, /**/);
        }

        /* Re-enable UUART and IRQs */
        USB_SERIAL_UART->CR1 |= USART_CR1_UE;
        __enable_irq();
    }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    if (dtr ^ rts) {
        /* In case any PTT is asserted, disable UART transmitter due to those sharing the same lines */
        __disable_irq();
        USB_SERIAL_UART->CR1 &= (uint32_t) ~USART_CR1_TE;
        __enable_irq();
    }

    if (dtr & !rts) {
        /* PTT1 */
        USB_SERIAL_UART_GPIO->BSRR |= USB_SERIAL_UART_PIN_PTT1;
        LED_SET(1, 1);
    } else {
        USB_SERIAL_UART_GPIO->BRR |= USB_SERIAL_UART_PIN_PTT1;
        LED_SET(1, 0);
    }

    if (!dtr & rts) {
        /* PTT2 */
        USB_SERIAL_UART_GPIO->BSRR |= USB_SERIAL_UART_PIN_PTT2;
        LED_SET(0, 1);
    } else {
        USB_SERIAL_UART_GPIO->BRR |= USB_SERIAL_UART_PIN_PTT2;
        LED_SET(0, 0);
    }

    if ( !(dtr ^ rts) ) {
        /* Enable UART transmitter again, when no PTT is asserted */
        __disable_irq();
        USB_SERIAL_UART->CR1 |= USART_CR1_TE;
        __enable_irq();
    }
}

void USB_SerialInit(void)
{
    /* Set up GPIO */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef SerialGpio;
    SerialGpio.Pin = (USB_SERIAL_UART_PIN_TX | USB_SERIAL_UART_PIN_RX);
    SerialGpio.Mode = GPIO_MODE_AF_PP;
    SerialGpio.Pull = GPIO_PULLUP;
    SerialGpio.Speed = GPIO_SPEED_FREQ_LOW;
    SerialGpio.Alternate = GPIO_AF7_USART1;
    HAL_GPIO_Init(USB_SERIAL_UART_GPIO, &SerialGpio);

    /* Set up RTS and DTR controlled GPIOs */
    GPIO_InitTypeDef RtsDtrGpio = {
        .Pin = (USB_SERIAL_UART_PIN_PTT2 | USB_SERIAL_UART_PIN_PTT1),
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_FREQ_LOW,
        .Alternate = 0
    };
    HAL_GPIO_Init(USB_SERIAL_UART_GPIO, &RtsDtrGpio);

    /* Initialize UART */
    __HAL_RCC_USART1_CLK_ENABLE();
    USB_SERIAL_UART->CR1 = USART_CR1_RTOIE | UART_OVERSAMPLING_16 | UART_WORDLENGTH_8B
            | UART_PARITY_NONE | USART_CR1_RXNEIE | UART_MODE_RX; /* Enable receiver only, transmitter will be enabled on-demand */
    USB_SERIAL_UART->CR2 = UART_RECEIVER_TIMEOUT_ENABLE | UART_STOPBITS_1;
    USB_SERIAL_UART->CR3 = USART_CR3_EIE;
    USB_SERIAL_UART->BRR = (HAL_RCCEx_GetPeriphCLKFreq(USB_SERIAL_UART_PERIPHCLK) + USB_SERIAL_UART_DEFBAUD/2) / USB_SERIAL_UART_DEFBAUD;
    USB_SERIAL_UART->RTOR = ((uint32_t) USB_SERIAL_UART_RXTIMEOUT << USART_RTOR_RTO_Pos) & USART_RTOR_RTO_Msk;
    USB_SERIAL_UART->CR1 |= USART_CR1_UE;

    /* Enable interrupt */
    NVIC_SetPriority(ADC1_2_IRQn, AIOC_IRQ_PRIO_SERIAL);
    NVIC_EnableIRQ(USART1_IRQn);
}

void USB_SerialTask(void)
{

}

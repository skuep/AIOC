#include <usb.h>
#include "stm32f3xx_hal.h"
#include "tusb.h"
#include "usb_serial.h"

// FIXME: Do all three need to be handled, or just the LP one?
// USB high-priority interrupt (Channel 74): Triggered only by a correct
// transfer event for isochronous and double-buffer bulk transfer to reach
// the highest possible transfer rate.
void USB_HP_IRQHandler(void)
{
  tud_int_handler(0);
}

// USB low-priority interrupt (Channel 75): Triggered by all USB events
// (Correct transfer, USB reset, etc.). The firmware has to check the
// interrupt source before serving the interrupt.
void USB_LP_IRQHandler(void)
{
  tud_int_handler(0);
}

// USB wakeup interrupt (Channel 76): Triggered by the wakeup event from the USB
// Suspend mode.
void USBWakeUp_RMP_IRQHandler(void)
{
  tud_int_handler(0);
}


void USB_Init(void)
{
    __HAL_REMAPINTERRUPT_USB_ENABLE();

    /* Configure USB DM and DP pins */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_USB;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Enable USB clock
    __HAL_RCC_USB_CLK_ENABLE();

    // Init classes
    USB_SerialInit();

    // Start USB Stack
    tud_init(BOARD_TUD_RHPORT);
}

void USB_Task(void)
{
    USB_SerialTask();
    tud_task();
}


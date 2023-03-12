#include "usb.h"
#include "stm32f3xx_hal.h"
#include "aioc.h"
#include "tusb.h"
#include "usb_serial.h"
#include "usb_audio.h"
#include "usb_hid.h"

// We have ISOCHRONOUS endpoints defined that share the same endpoint number, but have opposing directions.
// However with STM32 hardware, ISOCHRONOUS endpoints use both RX and TX structures of the same endpoint register in hardware
// We circumvent a clash by defining our own custom endpoint map for the tiny usb stack.
// This callback is probably not needed with new versions of tinyusb
uint8_t tu_stm32_edpt_number_cb(uint8_t addr)
{
    switch (addr) {
    case 0x00:
    case 0x80:
        /* Endpoint Zero */
        return 0x00;

    case EPNUM_AUDIO_IN:
        return 0x01;

    case EPNUM_AUDIO_OUT:
        return 0x02;

    case EPNUM_AUDIO_FB:
        return 0x03;

    case EPNUM_HID_IN:
    case EPNUM_HID_OUT:
        return 0x04;

    case EPNUM_CDC_0_OUT:
    case EPNUM_CDC_0_IN:
        return 0x05;

    case EPNUM_CDC_0_NOTIF:
        return 0x06;

    default:
        TU_BREAKPOINT();
        return 0x00;
    }
}

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

// Invoked when device is mounted (configured)
void tud_mount_cb(void)
{

}

// Invoked when device is unmounted
void tud_umount_cb(void)
{

}

// Invoked when usb bus is suspended
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{

}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{

}

void Timer_Init(void)
{
    __HAL_RCC_TIM2_CLK_ENABLE();

    /* TIM2 generates a timebase for USB OUT feedback endpoint */
    USB_SOF_TIMER->CR1 = TIM_CLOCKDIVISION_DIV1 | TIM_COUNTERMODE_UP | TIM_AUTORELOAD_PRELOAD_ENABLE;
    USB_SOF_TIMER->PSC = 0;
    USB_SOF_TIMER->ARR = 0xFFFFFFFFUL;
    USB_SOF_TIMER->CCMR1 = (0x1 << TIM_CCMR1_CC1S_Pos);
    USB_SOF_TIMER->EGR = TIM_EGR_UG;
    USB_SOF_TIMER->CR1 |= TIM_CR1_CEN;

    TU_ASSERT((2 * HAL_RCC_GetPCLK1Freq()) == USB_SOF_TIMER_HZ, /**/);
}

void GPIO_Init(void)
{
    /* Configure USB DM and DP pins */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = (GPIO_PIN_11 | GPIO_PIN_12);
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF14_USB;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void USB_Reset(void)
{
    /* pull USB DP pins low to simulate disconnect
       to force the host to re-enumerate when a new program is loaded */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIOA->BRR = GPIO_PIN_12;
}

void USB_Init(void)
{
    __HAL_REMAPINTERRUPT_USB_ENABLE();
    __HAL_RCC_USB_CLK_ENABLE();

    GPIO_Init();
    Timer_Init();

    // Init classes
    USB_SerialInit();
    USB_AudioInit();
    USB_HIDInit();

    // Start USB Stack
    tud_init(BOARD_TUD_RHPORT);

    NVIC_SetPriority(USB_LP_IRQn, AIOC_IRQ_PRIO_USB);
    NVIC_SetPriority(USB_HP_IRQn, AIOC_IRQ_PRIO_USB);
}

void USB_Task(void)
{
    USB_SerialTask();
    tud_task();
}


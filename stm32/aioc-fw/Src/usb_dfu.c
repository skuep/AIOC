#include "usb_dfu.h"
#include "stm32f3xx_hal.h"
#include "tusb.h"
#include "usb.h"

// Invoked on DFU_DETACH request to reboot to the bootloader
void tud_dfu_runtime_reboot_to_dfu_cb(void)
{
    /* Reset USB and force reset via watchdog timer. */
    USB_Reset();

    IWDG_HandleTypeDef IWDGHandle = {
        .Instance = IWDG,
        .Init = {
            .Prescaler = IWDG_PRESCALER_4,
            .Reload = 128,
            .Window = 0x0FFF
        }
    };

    HAL_IWDG_Init(&IWDGHandle);

    while(1) {
        /* Wait for Reset */
    }
}

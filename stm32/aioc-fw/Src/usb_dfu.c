#include "usb_dfu.h"
#include "stm32f3xx_hal.h"
#include "tusb.h"

// Invoked on DFU_DETACH request to reboot to the bootloader
void tud_dfu_runtime_reboot_to_dfu_cb(void)
{
    /* Use the watchdog timer for a timed reset. This assures
     * the host can close the open pipe before the device reboots. */
    __HAL_RCC_WWDG_CLK_ENABLE();

    WWDG_HandleTypeDef WWDGHandle = {
        .Instance = WWDG,
        .Init = {
            .Prescaler = WWDG_PRESCALER_1,
            .Window = 0x7F,
			.Counter = 0x50,
			.EWIMode = WWDG_EWI_DISABLE
        }
    };

    HAL_WWDG_Init(&WWDGHandle);
}

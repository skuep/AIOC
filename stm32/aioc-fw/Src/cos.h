#ifndef COS_H_
#define COS_H_

#include <stdint.h>
#include "stm32f3xx_hal.h"
#include "usb_hid.h"

static inline void COS_SetState(uint8_t state)
{
    LED_SET(0, state & 0x01 ? 1 : 0);

    USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_VOLDN : 0x00);
    USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_DCD : 0x00);
}

#endif /* COS_H_ */

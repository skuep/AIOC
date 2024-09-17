#ifndef COS_H_
#define COS_H_

#include <stdint.h>
#include "stm32f3xx_hal.h"
#include "usb_hid.h"
#include "usb_serial.h"
#include "settings.h"

static inline void COS_VirtualSetState(uint8_t state)
{
    LED_SET(0, state & 0x01 ? 1 : 0);

    if (settingsRegMap[SETTINGS_REG_CM108_IOMUX0] & SETTINGS_REG_CM108_IOMUX0_BTN1SRC_VCOS_MASK) {
        USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_VOLUP : 0x00);
    }

    if (settingsRegMap[SETTINGS_REG_CM108_IOMUX1] & SETTINGS_REG_CM108_IOMUX1_BTN2SRC_VCOS_MASK) {
        USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_VOLDN : 0x00);
    }

    if (settingsRegMap[SETTINGS_REG_CM108_IOMUX2] & SETTINGS_REG_CM108_IOMUX2_BTN3SRC_VCOS_MASK) {
        USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_PLAYMUTE : 0x00);
    }

    if (settingsRegMap[SETTINGS_REG_CM108_IOMUX3] & SETTINGS_REG_CM108_IOMUX3_BTN4SRC_VCOS_MASK) {
        USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_RECMUTE : 0x00);
    }

    if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX0] & SETTINGS_REG_SERIAL_IOMUX0_DCDSRC_VCOS_MASK) {
        USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_DCD : 0x00);
    }

    if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX1] & SETTINGS_REG_SERIAL_IOMUX1_DSRSRC_VCOS_MASK) {
        USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_DSR : 0x00);
    }

    if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX2] & SETTINGS_REG_SERIAL_IOMUX2_RISRC_VCOS_MASK) {
        USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_RI : 0x00);
    }

    if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX3] & SETTINGS_REG_SERIAL_IOMUX3_BRKSRC_VCOS_MASK) {
        USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_BREAK : 0x00);
    }
}

#endif /* COS_H_ */

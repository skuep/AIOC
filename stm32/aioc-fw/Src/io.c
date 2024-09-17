#include "io.h"
#include "usb_hid.h"
#include "usb_serial.h"
#include "settings.h"

void IO_IN_EXTI_ISR(void)
{
    if (EXTI->PR & IO_IN_PIN_1_EXTI_PR) {
        uint8_t state = IO_IN_GPIO->IDR & IO_IN_PIN_1 ? 0x00 : 0x01;

        if (settingsRegMap[SETTINGS_REG_CM108_IOMUX0] & SETTINGS_REG_CM108_IOMUX0_BTN1SRC_IN1_MASK) {
            USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_VOLUP : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_CM108_IOMUX1] & SETTINGS_REG_CM108_IOMUX1_BTN2SRC_IN1_MASK) {
            USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_VOLDN : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_CM108_IOMUX2] & SETTINGS_REG_CM108_IOMUX2_BTN3SRC_IN1_MASK) {
            USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_PLAYMUTE : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_CM108_IOMUX3] & SETTINGS_REG_CM108_IOMUX3_BTN4SRC_IN1_MASK) {
            USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_RECMUTE : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX0] & SETTINGS_REG_SERIAL_IOMUX0_DCDSRC_IN1_MASK) {
            USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_DCD : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX1] & SETTINGS_REG_SERIAL_IOMUX1_DSRSRC_IN1_MASK) {
            USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_DSR : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX2] & SETTINGS_REG_SERIAL_IOMUX2_RISRC_IN1_MASK) {
            USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_RI : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX3] & SETTINGS_REG_SERIAL_IOMUX3_BRKSRC_IN1_MASK) {
            USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_BREAK : 0x00);
        }
    }

    if (EXTI->PR & IO_IN_PIN_2_EXTI_PR) {
        uint8_t state = IO_IN_GPIO->IDR & IO_IN_PIN_2 ? 0x00 : 0x01;

        if (settingsRegMap[SETTINGS_REG_CM108_IOMUX0] & SETTINGS_REG_CM108_IOMUX0_BTN1SRC_IN2_MASK) {
            USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_VOLUP : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_CM108_IOMUX1] & SETTINGS_REG_CM108_IOMUX1_BTN2SRC_IN2_MASK) {
            USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_VOLDN : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_CM108_IOMUX2] & SETTINGS_REG_CM108_IOMUX2_BTN3SRC_IN2_MASK) {
            USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_PLAYMUTE : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_CM108_IOMUX3] & SETTINGS_REG_CM108_IOMUX3_BTN4SRC_IN2_MASK) {
            USB_HIDSendButtonState(state & 0x01 ? USB_HID_BUTTON_RECMUTE : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX0] & SETTINGS_REG_SERIAL_IOMUX0_DCDSRC_IN2_MASK) {
            USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_DCD : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX1] & SETTINGS_REG_SERIAL_IOMUX1_DSRSRC_IN2_MASK) {
            USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_DSR : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX2] & SETTINGS_REG_SERIAL_IOMUX2_RISRC_IN2_MASK) {
            USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_RI : 0x00);
        }

        if (settingsRegMap[SETTINGS_REG_SERIAL_IOMUX3] & SETTINGS_REG_SERIAL_IOMUX3_BRKSRC_IN2_MASK) {
            USB_SerialSendLineState(state & 0x01 ? USB_SERIAL_LINESTATE_BREAK : 0x00);
        }
    }

    /* Clear flags */
    EXTI->PR = IO_IN_PIN_1_EXTI_PR | IO_IN_PIN_2_EXTI_PR;
}

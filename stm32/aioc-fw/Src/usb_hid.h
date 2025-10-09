#ifndef USB_HID_H_
#define USB_HID_H_

#include <stdint.h>
#include <stdbool.h>

#define USB_HID_BUTTON_VOLUP    0x01
#define USB_HID_BUTTON_VOLDN    0x02
#define USB_HID_BUTTON_PLAYMUTE 0x04
#define USB_HID_BUTTON_RECMUTE  0x08

void USB_HIDInit(void);
bool USB_HIDSendButtonState(uint8_t inputsMask);

#endif /* USB_HID_H_ */

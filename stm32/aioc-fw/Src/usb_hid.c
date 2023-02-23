#include "usb_hid.h"
#include "tusb.h"
#include "ptt.h"
#include "usb_descriptors.h"

#define USB_HID_INOUT_REPORT_LEN  4

static uint8_t gpioState = 0x00;

static void ControlPTT(uint8_t gpio)
{
    /* PTT1 on GPIO 3, PTT2 on GPIO4 */
    uint8_t pttMask = (gpio & 0x04 ? PTT_MASK_PTT1 : 0) |
                      (gpio & 0x08 ? PTT_MASK_PTT2 : 0);

    PTT_Control(pttMask);
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) itf;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  buffer[0] = 0x00;
  buffer[1] = gpioState;
  buffer[2] = 0x00;
  buffer[3] = 0x00;

  return USB_HID_INOUT_REPORT_LEN;
}

// Invoked when received SET_REPORT control request
void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    (void) itf;
    (void) report_id;

    switch (report_type) {
        case HID_REPORT_TYPE_OUTPUT:
            TU_ASSERT(bufsize == USB_HID_INOUT_REPORT_LEN, /* */);

            /* Output report, emulate CM108 behaviour */
            if ((buffer[0] & 0xC0) == 0) {
                gpioState = buffer[1];
                ControlPTT(gpioState);
            }
            break;

        case HID_REPORT_TYPE_FEATURE:
            /* Custom extension for configuring the AIOC */
            break;

        default:
            TU_BREAKPOINT();
            break;
    }
}


void USB_HIDInit(void)
{


}

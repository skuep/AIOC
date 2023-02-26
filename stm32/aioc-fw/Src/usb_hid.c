#include "usb_hid.h"
#include "tusb.h"
#include "ptt.h"
#include "usb_descriptors.h"

#define USB_HID_INOUT_REPORT_LEN  4

static uint8_t gpioState = 0x00;

static void MakeReport(uint8_t * buffer)
{
    /* TODO: Read the actual states of the GPIO input hardware pins. */
    buffer[0] = 0x00;
    buffer[1] = gpioState;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
}

static void SendReport(void)
{
    uint8_t reportBuffer[USB_HID_INOUT_REPORT_LEN];
    MakeReport(reportBuffer);

    tud_hid_report(0, reportBuffer, sizeof(reportBuffer));
}

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
  (void) buffer;
  (void) reqlen;

  switch (report_type) {
      case HID_REPORT_TYPE_INPUT:
          TU_ASSERT(reqlen >= USB_HID_INOUT_REPORT_LEN, 0);

          MakeReport(buffer);
          return USB_HID_INOUT_REPORT_LEN;

      case HID_REPORT_TYPE_FEATURE:
          /* Custom extension for configuring the AIOC */
          break;

      default:
          TU_BREAKPOINT();
          break;
  }

  return 0;
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
                uint8_t gpioChange = gpioState ^ buffer[1];
                gpioState = buffer[1];
                ControlPTT(gpioState);

                /* The CM108 sends an input report via the HID interrupt endpoint to the host,
                 * whenever something changes (e.g. GPIO). Currently, this is only the case, whenever
                 * the host changes the GPIO state (since the GPIOs are hardwired as output).
                 * In the future, we might also support GPIO as input for other purposes. In that
                 * case, we might need some kind of interrupt mechanism to detect changes on the GPIOs.
                 * However some GPIOs might be shared (e.g. with UART), so we need to find a way to
                 * only enable the external GPIO interrupts, if the host actually has opened the HID
                 * interrupt pipe.
                 */
                if (gpioChange) {
                    SendReport();
                }
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

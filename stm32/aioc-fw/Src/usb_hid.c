#include <io.h>
#include "usb_hid.h"
#include "tusb.h"
#include "settings.h"
#include "usb_descriptors.h"

#define USB_HID_INOUT_REPORT_LEN  4
#define USB_HID_FEATURE_REPORT_LEN 6

static uint8_t buttonState = 0x00;
static uint8_t gpioState = 0x00;
static uint8_t currentAddress = 0x0000;

static void MakeReport(uint8_t * buffer)
{
    /* TODO: Read the actual states of the GPIO input hardware pins. */
    buffer[0] = buttonState & 0x0F;
    buffer[1] = gpioState;
    buffer[2] = 0x00;
    buffer[3] = 0x00;
}

static bool SendReport(void)
{
    uint8_t reportBuffer[USB_HID_INOUT_REPORT_LEN];
    MakeReport(reportBuffer);

    return tud_hid_report(0, reportBuffer, sizeof(reportBuffer));
}

static void ControlPTT(uint8_t gpio)
{
    uint8_t pttMask = IO_PTT_MASK_NONE;

    if (settingsRegMap[SETTINGS_REG_AIOC_IOMUX0] & SETTINGS_REG_AIOC_IOMUX0_OUT1SRC_CM108GPIO1_MASK) {
        pttMask |= gpio & 0x01 ? IO_PTT_MASK_PTT1 : 0;
    }

    if (settingsRegMap[SETTINGS_REG_AIOC_IOMUX0] & SETTINGS_REG_AIOC_IOMUX0_OUT1SRC_CM108GPIO2_MASK) {
        pttMask |= gpio & 0x02 ? IO_PTT_MASK_PTT1 : 0;
    }

    if (settingsRegMap[SETTINGS_REG_AIOC_IOMUX0] & SETTINGS_REG_AIOC_IOMUX0_OUT1SRC_CM108GPIO3_MASK) {
        pttMask |= gpio & 0x04 ? IO_PTT_MASK_PTT1 : 0;
    }

    if (settingsRegMap[SETTINGS_REG_AIOC_IOMUX0] & SETTINGS_REG_AIOC_IOMUX0_OUT1SRC_CM108GPIO4_MASK) {
        pttMask |= gpio & 0x08 ? IO_PTT_MASK_PTT1 : 0;
    }

    if (settingsRegMap[SETTINGS_REG_AIOC_IOMUX1] & SETTINGS_REG_AIOC_IOMUX1_OUT2SRC_CM108GPIO1_MASK) {
        pttMask |= gpio & 0x01 ? IO_PTT_MASK_PTT2 : 0;
    }

    if (settingsRegMap[SETTINGS_REG_AIOC_IOMUX1] & SETTINGS_REG_AIOC_IOMUX1_OUT2SRC_CM108GPIO2_MASK) {
        pttMask |= gpio & 0x02 ? IO_PTT_MASK_PTT2 : 0;
    }

    if (settingsRegMap[SETTINGS_REG_AIOC_IOMUX1] & SETTINGS_REG_AIOC_IOMUX1_OUT2SRC_CM108GPIO3_MASK) {
        pttMask |= gpio & 0x04 ? IO_PTT_MASK_PTT2 : 0;
    }

    if (settingsRegMap[SETTINGS_REG_AIOC_IOMUX1] & SETTINGS_REG_AIOC_IOMUX1_OUT2SRC_CM108GPIO4_MASK) {
        pttMask |= gpio & 0x08 ? IO_PTT_MASK_PTT2 : 0;
    }

    IO_PTTControl(pttMask);
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
            TU_ASSERT(reqlen >= USB_HID_FEATURE_REPORT_LEN, 0);
            uint32_t data;
            Settings_RegRead(currentAddress, &data);

            buffer[0] = 0x00;
            buffer[1] = currentAddress;
            buffer[2] = (uint8_t) (data >> 0);
            buffer[3] = (uint8_t) (data >> 8);
            buffer[4] = (uint8_t) (data >> 16);
            buffer[5] = (uint8_t) (data >> 24);

            return reqlen;

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
            TU_ASSERT(bufsize == USB_HID_FEATURE_REPORT_LEN, /* */);
            uint8_t ctrlWord = ((((uint8_t)  buffer[0]) <<  0) );
            uint8_t address =  ((((uint8_t)  buffer[1]) <<  0) );
            uint32_t data = (   (((uint32_t) buffer[2]) <<  0) |
                                (((uint32_t) buffer[3]) <<  8) |
                                (((uint32_t) buffer[4]) << 16) |
                                (((uint32_t) buffer[5]) << 24) );



            if (ctrlWord & 0x10UL) {
                Settings_Default();
            }

            if (ctrlWord & 0x40UL) {
                Settings_Recall();
            }

            if (ctrlWord & 0x01UL) {
                /* Write strobe */
                Settings_RegWrite(address, data);
            }

            if (ctrlWord & 0x80UL) {
                Settings_Store();
            }

            currentAddress = address;
            break;

        default:
            TU_BREAKPOINT();
            break;
    }
}


void USB_HIDInit(void)
{


}

bool USB_HIDSendButtonState(uint8_t buttonMask)
{
    buttonState = buttonMask;

    return SendReport();
}

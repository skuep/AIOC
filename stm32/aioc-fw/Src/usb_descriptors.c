/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "tusb.h"
#include "usb_descriptors.h"
#include "settings.h"
#include "stm32f3xx_hal.h"

//--------------------------------------------------------------------+
// Device Descriptors
//--------------------------------------------------------------------+
tusb_desc_device_t desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = USB_BCD,
    .bDeviceClass       = TUSB_CLASS_MISC,
    .bDeviceSubClass    = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol    = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor           = USB_VID,
    .idProduct          = USB_PID,
    .bcdDevice          = 0x0100,
    .iManufacturer      = STR_IDX_MANUFACTURER,
    .iProduct           = STR_IDX_PRODUCT,
    .iSerialNumber      = STR_IDX_SERIAL,
    .bNumConfigurations = 0x01
};

// Invoked when received GET DEVICE DESCRIPTOR
// Application return pointer to descriptor
uint8_t const* tud_descriptor_device_cb(void) {
    /* Get up-to-date USB settings */
    desc_device.idVendor  = (settingsRegMap[SETTINGS_REG_USBID] & SETTINGS_REG_USBID_VID_MASK) >> SETTINGS_REG_USBID_VID_OFFS;
    desc_device.idProduct = (settingsRegMap[SETTINGS_REG_USBID] & SETTINGS_REG_USBID_PID_MASK) >> SETTINGS_REG_USBID_PID_OFFS;

    return (uint8_t const*) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const desc_hid_report[] = {
    /* CM108 emulation. */
    HID_USAGE_PAGE   ( HID_USAGE_PAGE_CONSUMER ),
    HID_USAGE        ( HID_USAGE_CONSUMER_CONTROL ),
    HID_COLLECTION   ( HID_COLLECTION_APPLICATION ),
      /* Volume Up/Dn */
      HID_LOGICAL_MIN ( 0x00                                    ),
      HID_LOGICAL_MAX ( 0x01                                    ),
      HID_USAGE       ( 0x00 /* Unassigned */                   ),
      HID_USAGE       ( 0x00 /* Unassigned */                   ),
      HID_REPORT_SIZE ( 1                                       ),
      HID_REPORT_COUNT( 2                                       ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ),
      /* Mute */
      HID_USAGE       ( 0x00 /* Unassigned */                   ),
      HID_USAGE       ( 0x00 /* Unassigned */                   ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_RELATIVE  ),
      /* Hook Switch */
      HID_USAGE_PAGE  ( HID_USAGE_PAGE_TELEPHONY                ),
      HID_USAGE       ( 0x00 /* Unassigned */                   ),
      HID_REPORT_COUNT( 1                                       ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE | HID_NULL_STATE),
      /* Filler */
      HID_USAGE_PAGE  ( HID_USAGE_PAGE_CONSUMER                 ),
      HID_USAGE       ( 0x00 /* Unassigned */                   ),
      HID_REPORT_COUNT( 3                                       ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ),
      /* GPIO and Status */
      HID_LOGICAL_MAX_N ( 0xFF, 2                               ),
      HID_USAGE       ( 0x00 /* Unassigned */                   ),
      HID_REPORT_SIZE ( 8                                       ),
      HID_REPORT_COUNT( 3                                       ),
      HID_INPUT       ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ),
      /* Output */
      HID_USAGE       ( 0x00 /* Unassigned */                   ),
      HID_REPORT_COUNT( 4                                       ),
      HID_OUTPUT      ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ),
      /* Feature for configuring AIOC */
      HID_USAGE       ( 0x00 /* Unassigned */                   ),
      HID_REPORT_COUNT( 6                                       ),
      HID_FEATURE     ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE  ),
    HID_COLLECTION_END
};

// Invoked when received GET HID REPORT DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const * tud_hid_descriptor_report_cb(uint8_t itf)
{
    (void) itf;
    return desc_hid_report;
}

//--------------------------------------------------------------------+
// Configuration Descriptor
//--------------------------------------------------------------------+
#define CONFIG_TOTAL_LEN ( \
        TUD_CONFIG_DESC_LEN + \
        AIOC_AUDIO_DESC_LEN + \
        AIOC_HID_DESC_LEN + \
        AIOC_CDC_DESC_LEN + \
        AIOC_DFU_RT_DESC_LEN \
)

uint8_t const desc_fs_configuration[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(
        /* config_num */    1,
        /* _itfcount */     ITF_NUM_TOTAL,
        /* _stridx */       0x00,
        /* _total_len */    CONFIG_TOTAL_LEN,
        /* _attribute */    0x00,
        /* _power_ma */     100
    ),

    AIOC_AUDIO_DESCRIPTOR(
        /* _itfnum */       ITF_NUM_AUDIO_CONTROL,
        /* _stridx */       STR_IDX_AUDIOITF,
        /* _nBytesPerSample */ CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE,
        /* _nBitsUsedPerSample */ CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE*8,
        /* _epin */         EPNUM_AUDIO_IN,
        /* _epinsize */     CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX,
        /* _epout */        EPNUM_AUDIO_OUT,
        /* _epoutsize */    CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX,
        /* _epfb */         EPNUM_AUDIO_FB
    ),
    AIOC_HID_DESCRIPTOR(
        /* _itfnum */       ITF_NUM_HID,
        /* _stridx */       STR_IDX_HIDITF,
        /* _boot_protocol */HID_ITF_PROTOCOL_NONE,
        /*_report_desc_len*/sizeof(desc_hid_report),
        /* _epin */         EPNUM_HID_IN,
        /* _epsize */       CFG_TUD_HID_EP_BUFSIZE,
        /* _ep_interval */  0x20
    ),

    AIOC_CDC_DESCRIPTOR(
        /* _itfnum */       ITF_NUM_CDC_0,
        /* _stridx */       STR_IDX_CDCITF,
        /* _ep_notif */     EPNUM_CDC_0_NOTIF,
        /* _ep_notif_size */ 8,
        /* _epout */        EPNUM_CDC_0_OUT,
        /* _epin */         EPNUM_CDC_0_IN,
        /* _epsize */       CFG_TUD_CDC_EP_BUFSIZE
    ),

    AIOC_DFU_RT_DESCRIPTOR(
        /* _itfnum */       ITF_NUM_DFU_RT,
        /* _stridx */       STR_IDX_DFU_RT,
        /* _attr */         DFU_ATTR_WILL_DETACH | \
                            DFU_ATTR_CAN_UPLOAD  | \
                            DFU_ATTR_CAN_DOWNLOAD,
        /* _timeout */      255, /* not used if WILL_DETACH */
        /* _xfer_size */    2048 /* max size for stm32 dfu bootloader */
    )
};

// Invoked when received GET CONFIGURATION DESCRIPTOR
// Application return pointer to descriptor
// Descriptor contents must exist long enough for transfer to complete
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void) index; // for multiple configurations

    TU_ASSERT(!TUD_OPT_HIGH_SPEED);
    return desc_fs_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+
static uint32_t calc_crc32(const uint8_t *buf_ptr, uint32_t buf_len) {
    /* Generate a serial number from processor UID using CRC32 */
    const uint32_t crc32_init = 0xFFFFFFFFUL;
    const uint32_t crc32_poly = 0xEDB88320UL;
    const uint32_t crc32_final = 0xFFFFFFFFUL;
    uint32_t serial = crc32_init; /* CRC32 Initial value */

    while (buf_len-- > 0) {
        uint8_t byte = *buf_ptr++;
        serial = serial ^ byte;
        for (uint8_t j = 0; j < 8; j++) {
            uint32_t mask = -(serial & 1);
            serial = (serial >> 1) ^ (crc32_poly & mask);
        }
    }

    return serial ^ crc32_final;
}

static const char * get_serial(void) {
    static char serial_str[sizeof(uint32_t) * 2 + 1];
    uint32_t serial_num = calc_crc32((uint8_t *) UID_BASE, 12);

    for (uint8_t i = 0; i < (sizeof(uint32_t) * 2); i++) {
        uint8_t nibble = ((serial_num & 0xF0000000UL) >> 28);
        serial_str[i] = nibble < 0xA ? nibble + '0' : nibble - 0xA + 'a';
        serial_num <<= 4;
    }

    return serial_str;
}

static uint8_t ascii_to_utf16(uint8_t *buffer, uint32_t size, const char *str) {
    uint8_t len = 0;

    while ((*str != '\0') && (size > 0)) {
        *buffer++ = *str++;
        *buffer++ = 0x00;
        size -= 2;
        len += 2;
    }

    return len;
}

// Invoked when received GET STRING DESCRIPTOR request
// Application return pointer to descriptor, whose contents must exist long enough for transfer to complete
const uint16_t * tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    static uint8_t buffer[64];
    uint8_t *ptr = &buffer[2];
    uint8_t len = sizeof(buffer) - 2;

    (void) langid;

    switch (index) {
    case STR_IDX_LANGUAGE:
        ptr[0] = 0x09;
        ptr[1] = 0x04;
        len = 2;
        break;

    case STR_IDX_MANUFACTURER:
        len = ascii_to_utf16(ptr, len, USB_STRING_MANUFACTURER);
        break;

    case STR_IDX_PRODUCT:
        len = ascii_to_utf16(ptr, len, USB_STRING_PRODUCT);
        break;

    case STR_IDX_SERIAL:
        len = ascii_to_utf16(ptr, len, get_serial());
        break;

    case STR_IDX_CDCITF:
        len = ascii_to_utf16(ptr, len, USB_STRING_CDCITF);
        break;

    case STR_IDX_AUDIOITF:
        len = ascii_to_utf16(ptr, len, USB_STRING_AUDIOITF);
        break;

    case STR_IDX_AUDIOIN:
        len = ascii_to_utf16(ptr, len, USB_STRING_AUDIOIN);
        break;

    case STR_IDX_AUDIOOUT:
        len = ascii_to_utf16(ptr, len, USB_STRING_AUDIOOUT);
        break;

    case STR_IDX_AUDIOINVOL:
        len = ascii_to_utf16(ptr, len, USB_STRING_AUDIOINVOL);
        break;

    case STR_IDX_AUDIOOUTVOL:
        len = ascii_to_utf16(ptr, len, USB_STRING_AUDIOOUTVOL);
        break;

    case STR_IDX_AUDIOINCHAN:
        len = ascii_to_utf16(ptr, len, USB_STRING_AUDIOINCHAN);
        break;

    case STR_IDX_AUDIOOUTCHAN:
        len = ascii_to_utf16(ptr, len, USB_STRING_AUDIOOUTCHAN);
        break;

    case STR_IDX_HIDITF:
        len = ascii_to_utf16(ptr, len, USB_STRING_HIDITF);
        break;

    case STR_IDX_DFU_RT:
        len = ascii_to_utf16(ptr, len, USB_STRING_DFU_RT);
        break;

    default:
        TU_ASSERT(0, NULL);
        break;
    }

    buffer[0] = len + 2;
    buffer[1] = TUSB_DESC_STRING;

    return (void *) buffer;
}

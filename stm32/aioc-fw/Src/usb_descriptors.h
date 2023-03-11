#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

/* Interfaces */
enum USB_DESCRIPTORS_ITF {
    ITF_NUM_AUDIO_CONTROL = 0,
    ITF_NUM_AUDIO_STREAMING_OUT,
    ITF_NUM_AUDIO_STREAMING_IN,
    ITF_NUM_HID, /* For CM108 compatibility make this interface #3 */
    ITF_NUM_CDC_0,
    ITF_NUM_CDC_0_DATA,
    ITF_NUM_DFU_RT,
    ITF_NUM_TOTAL
};

/* Strings */
enum USB_STRING_IDX {
    STR_IDX_LANGUAGE = 0,
    STR_IDX_MANUFACTURER,
    STR_IDX_PRODUCT,
    STR_IDX_SERIAL,
    STR_IDX_AUDIOITF,
    STR_IDX_AUDIOIN,
    STR_IDX_AUDIOOUT,
    STR_IDX_AUDIOINVOL,
    STR_IDX_AUDIOOUTVOL,
    STR_IDX_AUDIOINCHAN,
    STR_IDX_AUDIOOUTCHAN,
    STR_IDX_HIDITF,
    STR_IDX_CDCITF,
    STR_IDX_DFU_RT
};

#define USB_VID                     0x1209
#define USB_PID                     0x7388
#define USB_BCD                     0x0200

#define USB_STRING_MANUFACTURER     "AIOC"
#define USB_STRING_PRODUCT          "All-In-One-Cable"
#define USB_STRING_AUDIOITF         "AIOC Audio"
#define USB_STRING_AUDIOIN          "AIOC Audio In"
#define USB_STRING_AUDIOOUT         "AIOC Audio Out"
#define USB_STRING_AUDIOINVOL       "AIOC Audio In Volume"
#define USB_STRING_AUDIOOUTVOL      "AIOC Audio Out Volume"
#define USB_STRING_AUDIOINCHAN      "AIOC Audio In Channel"
#define USB_STRING_AUDIOOUTCHAN     "AIOC Audio Out Channel"
#define USB_STRING_CDCITF           "AIOC CDC"
#define USB_STRING_HIDITF           "AIOC HID"
#define USB_STRING_DFU_RT           "AIOC DFU Runtime"

/* Endpoints */
#define EPNUM_AUDIO_IN      0x81
#define EPNUM_AUDIO_OUT     0x02
#define EPNUM_AUDIO_FB      0x82
#define EPNUM_HID_IN        0x83
#define EPNUM_HID_OUT       0x03
#define EPNUM_CDC_0_OUT     0x04
#define EPNUM_CDC_0_IN      0x84
#define EPNUM_CDC_0_NOTIF   0x85

/* Custom Audio Descriptor.
 * Courtesy of https://github.com/hathach/tinyusb/issues/1249#issuecomment-1148727765 */
#define AUDIO_CTRL_ID_SPK_INPUT_STREAM  0x01
#define AUDIO_CTRL_ID_SPK_FUNIT         0x02
#define AUDIO_CTRL_ID_SPK_OUTPUT        0x03
#define AUDIO_CTRL_ID_SPK_CLOCK         0x08
#define AUDIO_CTRL_ID_MIC_INPUT         0x11
#define AUDIO_CTRL_ID_MIC_FUNIT         0x12
#define AUDIO_CTRL_ID_MIC_OUTPUT_STREAM 0x13
#define AUDIO_CTRL_ID_MIC_CLOCK         0x18

#define AUDIO_NUM_INTERFACES        0x03
#define AUDIO_NUM_INCHANNELS        0x01
#define AUDIO_NUM_OUTCHANNELS       0x01

#define AIOC_AUDIO_CTRL_TOTAL_LEN ( \
    TUD_AUDIO_DESC_CLK_SRC_LEN + \
    TUD_AUDIO_DESC_INPUT_TERM_LEN + \
    TUD_AUDIO_DESC_FEATURE_UNIT_ONE_CHANNEL_LEN + \
    TUD_AUDIO_DESC_OUTPUT_TERM_LEN + \
    TUD_AUDIO_DESC_CLK_SRC_LEN + \
    TUD_AUDIO_DESC_INPUT_TERM_LEN + \
    TUD_AUDIO_DESC_OUTPUT_TERM_LEN + \
    TUD_AUDIO_DESC_FEATURE_UNIT_ONE_CHANNEL_LEN)

#define AIOC_AUDIO_DESC_LEN ( \
    TUD_AUDIO_DESC_IAD_LEN + \
    TUD_AUDIO_DESC_STD_AC_LEN + \
    TUD_AUDIO_DESC_CS_AC_LEN + \
    AIOC_AUDIO_CTRL_TOTAL_LEN + \
    /* Speaker Interface */ \
    TUD_AUDIO_DESC_STD_AS_INT_LEN + \
    TUD_AUDIO_DESC_STD_AS_INT_LEN + \
    TUD_AUDIO_DESC_CS_AS_INT_LEN + \
    TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN + \
    TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN + \
    TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN + \
    TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN + \
    /* Microphone Interface */ \
    TUD_AUDIO_DESC_STD_AS_INT_LEN + \
    TUD_AUDIO_DESC_STD_AS_INT_LEN + \
    TUD_AUDIO_DESC_CS_AS_INT_LEN + \
    TUD_AUDIO_DESC_TYPE_I_FORMAT_LEN + \
    TUD_AUDIO_DESC_STD_AS_ISO_EP_LEN + \
    TUD_AUDIO_DESC_CS_AS_ISO_EP_LEN)

#define AIOC_AUDIO_DESCRIPTOR(_itfnum, _stridx, _nBytesPerSample, _nBitsUsedPerSample, _epin, _epinsize, _epout, _epoutsize, _epfb) \
    /* Standard Interface Association Descriptor (IAD) */ \
    TUD_AUDIO_DESC_IAD(_itfnum, AUDIO_NUM_INTERFACES, /*_stridx*/ 0x00), \
    /* Audio Control Interface */ \
    /* Standard AC Interface Descriptor(4.7.1) */ \
    TUD_AUDIO_DESC_STD_AC(/*_itfnum*/ _itfnum, /*_nEPs*/ 0x00, /*_stridx*/ _stridx), \
    /* Class-Specific AC Interface Header Descriptor(4.7.2) */ \
    TUD_AUDIO_DESC_CS_AC(/*_bcdADC*/ 0x0200, AUDIO_FUNC_CONVERTER, AIOC_AUDIO_CTRL_TOTAL_LEN, /*_ctrl*/ AUDIO_CS_AS_INTERFACE_CTRL_LATENCY_POS), \
    /* Clock Source Descriptor(4.7.2.1) */ \
    TUD_AUDIO_DESC_CLK_SRC(AUDIO_CTRL_ID_SPK_CLOCK, AUDIO_CLOCK_SOURCE_ATT_INT_PRO_CLK, (AUDIO_CTRL_RW << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), /*_assocTerm*/ AUDIO_CTRL_ID_SPK_INPUT_STREAM, /*_stridx*/ 0x00), \
    /* Speaker Terminals */ \
    TUD_AUDIO_DESC_INPUT_TERM(/*_termid*/ AUDIO_CTRL_ID_SPK_INPUT_STREAM, /*_termtype*/ AUDIO_TERM_TYPE_USB_STREAMING, /*_assocTerm*/ AUDIO_CTRL_ID_MIC_OUTPUT_STREAM, /*_clkid*/ AUDIO_CTRL_ID_SPK_CLOCK, /*_nchannelslogical*/ AUDIO_NUM_OUTCHANNELS, /*_channelcfg*/ AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ STR_IDX_AUDIOOUTCHAN, /*_ctrl*/ AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, /*_stridx*/ STR_IDX_AUDIOOUT), \
    TUD_AUDIO_DESC_FEATURE_UNIT_ONE_CHANNEL(/*_unitid*/ AUDIO_CTRL_ID_SPK_FUNIT, /*_srcid*/ AUDIO_CTRL_ID_SPK_INPUT_STREAM, /*_ctrlch0master*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch1*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_stridx*/ STR_IDX_AUDIOOUTVOL),\
    TUD_AUDIO_DESC_OUTPUT_TERM(AUDIO_CTRL_ID_SPK_OUTPUT, AUDIO_TERM_TYPE_OUT_GENERIC_SPEAKER, AUDIO_CTRL_ID_SPK_INPUT_STREAM, AUDIO_CTRL_ID_SPK_FUNIT, AUDIO_CTRL_ID_SPK_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ STR_IDX_AUDIOOUT), \
    /* Clock Source Descriptor(4.7.2.1) */ \
    TUD_AUDIO_DESC_CLK_SRC(AUDIO_CTRL_ID_MIC_CLOCK, AUDIO_CLOCK_SOURCE_ATT_INT_PRO_CLK, (AUDIO_CTRL_RW << AUDIO_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), /*_assocTerm*/ AUDIO_CTRL_ID_MIC_INPUT, /*_stridx*/ 0x00), \
    /* Microphone Terminals */ \
    TUD_AUDIO_DESC_INPUT_TERM(AUDIO_CTRL_ID_MIC_INPUT, AUDIO_TERM_TYPE_IN_GENERIC_MIC, AUDIO_CTRL_ID_MIC_OUTPUT_STREAM, AUDIO_CTRL_ID_MIC_CLOCK, AUDIO_NUM_INCHANNELS, AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_idxchannelnames*/ STR_IDX_AUDIOINCHAN, /*_ctrl*/ AUDIO_CTRL_R << AUDIO_IN_TERM_CTRL_CONNECTOR_POS, /*_stridx*/ STR_IDX_AUDIOIN), \
    TUD_AUDIO_DESC_OUTPUT_TERM(AUDIO_CTRL_ID_MIC_OUTPUT_STREAM, AUDIO_TERM_TYPE_USB_STREAMING, AUDIO_CTRL_ID_MIC_INPUT, AUDIO_CTRL_ID_MIC_FUNIT, AUDIO_CTRL_ID_MIC_CLOCK, /*_ctrl*/ 0x0000, /*_stridx*/ STR_IDX_AUDIOINVOL), \
    TUD_AUDIO_DESC_FEATURE_UNIT_ONE_CHANNEL(/*_unitid*/ AUDIO_CTRL_ID_MIC_FUNIT, /*_srcid*/ AUDIO_CTRL_ID_MIC_INPUT, /*_ctrlch0master*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_ctrlch1*/ AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_MUTE_POS | AUDIO_CTRL_RW << AUDIO_FEATURE_UNIT_CTRL_VOLUME_POS, /*_stridx*/ STR_IDX_AUDIOIN),\
    /* Speaker Interface */ \
    /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */ \
    TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum) + 1), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00), \
    /* Interface 1, Alternate 1 - alternate interface for data streaming */ \
    TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum) + 1), /*_altset*/ 0x01, /*_nEPs*/ 0x02, /*_stridx*/ 0x00), \
    /* Class-Specific AS Interface Descriptor(4.9.2) */ \
    TUD_AUDIO_DESC_CS_AS_INT(AUDIO_CTRL_ID_SPK_INPUT_STREAM, AUDIO_CTRL_NONE, AUDIO_FORMAT_TYPE_I, AUDIO_DATA_FORMAT_TYPE_I_PCM, AUDIO_NUM_OUTCHANNELS, AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00), \
    /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */ \
    TUD_AUDIO_DESC_TYPE_I_FORMAT(_nBytesPerSample, _nBitsUsedPerSample), \
    /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */ \
    TUD_AUDIO_DESC_STD_AS_ISO_EP(_epout, (TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ASYNCHRONOUS | TUSB_ISO_EP_ATT_DATA), _epoutsize, TUD_OPT_HIGH_SPEED ? 0x04 : 0x01), \
    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */ \
    TUD_AUDIO_DESC_CS_AS_ISO_EP(AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, AUDIO_CTRL_NONE, AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, 0x0000), \
    /* Standard AS Isochronous Feedback Endpoint Descriptor(4.10.2.1) */ \
    TUD_AUDIO_DESC_STD_AS_ISO_FB_EP(_epfb, 1), \
    /* Microphone Interface */ \
    /* Standard AS Interface Descriptor(4.9.1) */ \
    /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */ \
    TUD_AUDIO_DESC_STD_AS_INT((uint8_t)((_itfnum)+2), /*_altset*/ 0x00, /*_nEPs*/ 0x00, /*_stridx*/ 0x00), \
    /* Standard AS Interface Descriptor(4.9.1) */ \
    /* Interface 1, Alternate 1 - alternate interface for data streaming */ \
    TUD_AUDIO_DESC_STD_AS_INT(/*_itfnum*/ (uint8_t)((_itfnum)+2), /*_altset*/ 0x01, /*_nEPs*/ 0x01, /*_stridx*/ 0x00), \
    /* Class-Specific AS Interface Descriptor(4.9.2) */ \
    TUD_AUDIO_DESC_CS_AS_INT(AUDIO_CTRL_ID_MIC_OUTPUT_STREAM, AUDIO_CTRL_NONE, AUDIO_FORMAT_TYPE_I, AUDIO_DATA_FORMAT_TYPE_I_PCM, AUDIO_NUM_INCHANNELS, AUDIO_CHANNEL_CONFIG_NON_PREDEFINED, /*_stridx*/ 0x00), \
    /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */ \
    TUD_AUDIO_DESC_TYPE_I_FORMAT(_nBytesPerSample, _nBitsUsedPerSample), \
    /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */ \
    TUD_AUDIO_DESC_STD_AS_ISO_EP(_epin, (TUSB_XFER_ISOCHRONOUS | TUSB_ISO_EP_ATT_ASYNCHRONOUS | TUSB_ISO_EP_ATT_DATA), _epinsize, TUD_OPT_HIGH_SPEED ? 0x04 : 0x01), \
    /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */ \
    TUD_AUDIO_DESC_CS_AS_ISO_EP(AUDIO_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, AUDIO_CTRL_NONE, AUDIO_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, 0x0000)


#define AIOC_HID_DESC_LEN    (9 + 9 + 7)

#define AIOC_HID_DESCRIPTOR(_itfnum, _stridx, _boot_protocol, _report_desc_len, _epin, _epsize, _ep_interval) \
  /* Interface */\
  9, TUSB_DESC_INTERFACE, _itfnum, 0, 1, TUSB_CLASS_HID, (uint8_t)((_boot_protocol) ? (uint8_t)HID_SUBCLASS_BOOT : 0), _boot_protocol, _stridx,\
  /* HID descriptor */\
  9, HID_DESC_TYPE_HID, U16_TO_U8S_LE(0x0100), 0, 1, HID_DESC_TYPE_REPORT, U16_TO_U8S_LE(_report_desc_len),\
  /* Endpoint In */\
  7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_epsize), _ep_interval

#define AIOC_CDC_DESC_LEN   TUD_CDC_DESC_LEN

#define AIOC_CDC_DESCRIPTOR TUD_CDC_DESCRIPTOR

#define AIOC_DFU_RT_DESC_LEN   TUD_DFU_RT_DESC_LEN

#define AIOC_DFU_RT_DESCRIPTOR TUD_DFU_RT_DESCRIPTOR

#endif /* USB_DESCRIPTORS_H_ */

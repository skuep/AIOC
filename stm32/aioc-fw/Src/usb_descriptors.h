#ifndef USB_DESCRIPTORS_H_
#define USB_DESCRIPTORS_H_

enum USB_DESCRIPTORS_ITF {
  ITF_NUM_CDC_0 = 0,
  ITF_NUM_CDC_0_DATA,
#if 0
  ITF_NUM_CDC_1,
  ITF_NUM_CDC_1_DATA,
#endif
  ITF_NUM_TOTAL
};

#define EPNUM_CDC_0_NOTIF   0x81
#define EPNUM_CDC_0_OUT     0x02
#define EPNUM_CDC_0_IN      0x82

#if 0
#define EPNUM_CDC_1_NOTIF   0x83
#define EPNUM_CDC_1_OUT     0x04
#define EPNUM_CDC_1_IN      0x84
#endif

#endif /* USB_DESCRIPTORS_H_ */

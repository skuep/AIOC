#ifndef USB_AUDIO_H_
#define USB_AUDIO_H_

#include <stdint.h>

typedef enum {
    USB_AUDIO_RXGAIN_1X,
    USB_AUDIO_RXGAIN_2X,
    USB_AUDIO_RXGAIN_4X,
    USB_AUDIO_RXGAIN_8X,
    USB_AUDIO_RXGAIN_16X,
} usb_audio_rxgain_t;

typedef enum {
    USB_AUDIO_TXBOOST_OFF,
    USB_AUDIO_TXBOOST_ON
} usb_audio_txboost_t;

typedef struct {
    uint16_t bufLevelMin;
    uint16_t bufLevelMax;
    uint16_t bufLevelAvg;
} usb_audio_bufstats_t;

typedef struct {
    uint32_t feedbackMin;
    uint32_t feedbackMax;
    uint32_t feedbackAvg;
} usb_audio_fbstats_t;

void USB_AudioInit(void);
void USB_AudioGetSpeakerFeedbackStats(usb_audio_fbstats_t * status);
void USB_AudioGetSpeakerBufferStats(usb_audio_bufstats_t * status);

#endif /* USB_AUDIO_H_ */

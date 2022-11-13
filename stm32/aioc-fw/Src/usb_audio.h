#ifndef USB_AUDIO_H_
#define USB_AUDIO_H_

#include <stdint.h>

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

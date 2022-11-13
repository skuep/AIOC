#ifndef USB_AUDIO_H_
#define USB_AUDIO_H_

#include <stdint.h>

void USB_AudioInit(void);

uint32_t USB_AudioGetSpeakerFeedback(void);

#endif /* USB_AUDIO_H_ */

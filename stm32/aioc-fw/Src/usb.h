#ifndef USB_H_
#define USB_H_

#include "usb_audio.h"
#include "usb_serial.h"
#include "usb_dfu.h"

#define USB_SOF_TIMER       TIM2
#define USB_SOF_TIMER_CNT   TIM2->CNT
#define USB_SOF_TIMER_HZ    72000000UL

void USB_Reset(void);
void USB_Init(void);
void USB_Task(void);

#endif /* USB_H_ */

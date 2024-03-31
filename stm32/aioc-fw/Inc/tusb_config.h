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

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+

// RHPort number used for device can be defined by board.mk, default to port 0
#ifndef BOARD_TUD_RHPORT
#define BOARD_TUD_RHPORT      0
#endif

// RHPort max operational speed can defined by board.mk
#ifndef BOARD_TUD_MAX_SPEED
#define BOARD_TUD_MAX_SPEED   OPT_MODE_FULL_SPEED
#endif

#ifndef MAX_EP_COUNT
#define MAX_EP_COUNT 8
#endif

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

// defined by board.mk
#ifndef CFG_TUSB_MCU
#error CFG_TUSB_MCU must be defined
#endif

#ifndef CFG_TUSB_OS
#define CFG_TUSB_OS           OPT_OS_NONE
#endif

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG        0
#endif

// Enable Device stack
#define CFG_TUD_ENABLED       1

// Default is max speed that hardware controller could support with on-chip PHY
#define CFG_TUD_MAX_SPEED     BOARD_TUD_MAX_SPEED

/* USB DMA on some MCUs can only access a specific SRAM region with restriction on alignment.
 * Tinyusb use follows macros to declare transferring memory so that they can be put
 * into those specific section.
 * e.g
 * - CFG_TUSB_MEM SECTION : __attribute__ (( section(".usb_ram") ))
 * - CFG_TUSB_MEM_ALIGN   : __attribute__ ((aligned(4)))
 */
#ifndef CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_SECTION
#endif

#ifndef CFG_TUSB_MEM_ALIGN
#define CFG_TUSB_MEM_ALIGN          __attribute__ ((aligned(4)))
#endif

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE    8
#endif

//------------- CLASS -------------//
#define CFG_TUD_AUDIO             1
#define CFG_TUD_CDC               1
#define CFG_TUD_HID               1
#define CFG_TUD_DFU_RUNTIME       1

// CDC FIFO size of TX and RX
#define CFG_TUD_CDC_RX_BUFSIZE   256
#define CFG_TUD_CDC_TX_BUFSIZE   256

// CDC Endpoint transfer buffer size, more is faster
#define CFG_TUD_CDC_EP_BUFSIZE   64

// HID buffer size
#define CFG_TUD_HID_EP_BUFSIZE   8

 //--------------------------------------------------------------------
 // AUDIO CLASS DRIVER CONFIGURATION
 //--------------------------------------------------------------------

 // Have a look into audio_device.h for all configurations

#define CFG_TUD_AUDIO_ENABLE_EP_IN                                    1
#define CFG_TUD_AUDIO_ENABLE_EP_OUT                                   1
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP                              1

#define CFG_TUD_AUDIO_FUNC_1_DESC_LEN                                 AIOC_AUDIO_DESC_LEN
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT                                 1
#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ                              64
#define CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE                       2
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX                            1
#define CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX                            1
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SZ_MAX                            CFG_TUD_AUDIO_EP_SZ_OUT + CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX
#define CFG_TUD_AUDIO_FUNC_1_EP_OUT_SW_BUF_SZ                         1024 /* FIFO not being power of 2 seem to result in all kinds of BusFaults? */
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX                             CFG_TUD_AUDIO_EP_SZ_IN + CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ                          1024

#define CFG_TUD_AUDIO_EP_SZ_IN                                        (48) * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX      // 48 Samples (48 kHz) x 2 Bytes/Sample x 1 Channel
#define CFG_TUD_AUDIO_EP_SZ_OUT                                       (48) * CFG_TUD_AUDIO_FUNC_1_N_BYTES_PER_SAMPLE * CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX      // 48 Samples (48 kHz) x 2 Bytes/Sample x 1 Channel

/* Include this here for pulling length of custom descriptors into tinyusb stack */
#include "usb_descriptors.h"

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */

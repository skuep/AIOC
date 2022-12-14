#include "usb_audio.h"
#include "stm32f3xx_hal.h"
#include "aioc.h"
#include "tusb.h"
#include "usb.h"
#include <math.h>

/* The one and only supported sample rate */
#define AUDIO_SAMPLE_RATE   48000
/* This is feedback average responsivity with a denominator of 65536 */
#define SPEAKER_FEEDBACK_AVG    32
/* This is buffer level average responsivity with a denominator of 65536 */
#define SPEAKER_BUFFERLVL_AVG   64
/* This is the amount of buffer level to feedback coupling with a denominator of 65536 to prevent buffer drift */
#define SPEAKER_BUFLVL_FB_COUPLING 1
/* We try to stay on this target with the buffer level */
#define SPEAKER_BUFFERLVL_TARGET (5 * CFG_TUD_AUDIO_EP_SZ_OUT) /* Keep our buffer at 5 frames, i.e. 5ms at full-speed USB */

typedef enum {
    STATE_OFF,
    STATE_START,
    STATE_RUN
} state_t;

/* Various state variables. N+1 because 0 is always the master channel */
static volatile state_t microphoneState = STATE_OFF;
static volatile state_t speakerState = STATE_OFF;
static bool microphoneMute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];
static bool speakerMute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1];
static int16_t microphoneLogVolume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1] = { [0 ... CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX] = 0 }; /* in dB */
static int16_t speakerLogVolume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1] = { [0 ... CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX] = 0 }; /* in dB */
static uint16_t microphoneLinVolume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1] = { [0 ... CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX] = 65535 }; /* 0.16 format */
static uint16_t speakerLinVolume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1] = { [0 ... CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX] = 65535 }; /* 0.16 format */
static uint64_t speakerFeedbackAvg; /* 32.32 format */
static uint32_t speakerFeedbackMin;
static uint32_t speakerFeedbackMax;
static uint32_t speakerBufferLvlAvg; /* 16.16 format */
static uint16_t speakerBufferLvlMin;
static uint16_t speakerBufferLvlMax;

//--------------------------------------------------------------------+
// Application Callback API Implementations
//--------------------------------------------------------------------+

// Invoked when audio class specific set request received for an entity
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff)
{
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  TU_ASSERT(itf == ITF_NUM_AUDIO_CONTROL);

  // We do not support any set range requests here, only current value requests
  TU_VERIFY(p_request->bRequest == AUDIO_CS_REQ_CUR);

  if ( entityID == AUDIO_CTRL_ID_MIC_FUNIT )
  {
    switch ( ctrlSel )
    {
      case AUDIO_FU_CTRL_MUTE:
        // Request uses format layout 1
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_1_t));

        microphoneMute[channelNum] = ((audio_control_cur_1_t*) pBuff)->bCur;

        TU_LOG2("    Set Mute: %d of channel: %u\r\n", microphoneMute[channelNum], channelNum);
      return true;

      case AUDIO_FU_CTRL_VOLUME:
        // Request uses format layout 2
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

        microphoneLogVolume[channelNum] = ((audio_control_cur_2_t*) pBuff)->bCur;
        double logVolume = microphoneLogVolume[channelNum] / 256; /* format is 7.8 fixed point */
        microphoneLinVolume[channelNum] = (microphoneLogVolume[channelNum] != 0x8000) ?
                (uint16_t) (65535 * pow(10, logVolume/20) + 0.5) : 0; /* log to linear with rounding */

        TU_LOG2("    Set Volume: %f dB of channel: %u\r\n", logVolume, channelNum);
      return true;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
      return false;
    }
  }

  if ( entityID == AUDIO_CTRL_ID_SPK_FUNIT )
  {
    switch ( ctrlSel )
    {
      case AUDIO_FU_CTRL_MUTE:
        // Request uses format layout 1
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_1_t));

        speakerMute[channelNum] = ((audio_control_cur_1_t*) pBuff)->bCur;

        TU_LOG2("    Set Mute: %d of channel: %u\r\n", speakerMute[channelNum], channelNum);

      return true;

      case AUDIO_FU_CTRL_VOLUME:
        // Request uses format layout 2
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

        speakerLogVolume[channelNum] = ((audio_control_cur_2_t*) pBuff)->bCur;
        double logVolume = (double) speakerLogVolume[channelNum] / 256; /* format is 7.8 fixed point */
        speakerLinVolume[channelNum] = (speakerLogVolume[channelNum] != 0x8000) ?
                (uint16_t) (65535 * pow(10, logVolume/20) + 0.5) : 0; /* log to linear with rounding */

        TU_LOG2("    Set Volume: %f dB of channel: %u\r\n", logVolume, channelNum);
      return true;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
      return false;
    }
  }

  return false;    // Yet not implemented
}

// Invoked when audio class specific get request received for an entity
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
  (void) rhport;

  // Page 91 in UAC2 specification
  uint8_t channelNum = TU_U16_LOW(p_request->wValue);
  uint8_t ctrlSel = TU_U16_HIGH(p_request->wValue);
  uint8_t itf = TU_U16_LOW(p_request->wIndex);
  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);

  TU_ASSERT(itf == ITF_NUM_AUDIO_CONTROL);

  // Input terminal (Microphone input)
  if (entityID == AUDIO_CTRL_ID_MIC_INPUT)
  {
    switch ( ctrlSel )
    {
      case AUDIO_TE_CTRL_CONNECTOR:
      {
        // The terminal connector control only has a get request with only the CUR attribute.
        audio_desc_channel_cluster_t ret;

        // Those are dummy values for now
        ret.bNrChannels = 1;
        ret.bmChannelConfig = 0;
        ret.iChannelNames = 0;

        TU_LOG2("    Get terminal connector\r\n");

        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*) &ret, sizeof(ret));
      }
      break;

        // Unknown/Unsupported control selector
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  // Output terminal (Speaker output)
  if (entityID == AUDIO_CTRL_ID_SPK_OUTPUT)
  {
    switch ( ctrlSel )
    {
      case AUDIO_TE_CTRL_CONNECTOR:
      {
        // The terminal connector control only has a get request with only the CUR attribute.
        audio_desc_channel_cluster_t ret;

        // Those are dummy values for now
        ret.bNrChannels = 1;
        ret.bmChannelConfig = 0;
        ret.iChannelNames = 0;

        TU_LOG2("    Get terminal connector\r\n");

        return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*) &ret, sizeof(ret));
      }
      break;

        // Unknown/Unsupported control selector
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  if (entityID == AUDIO_CTRL_ID_SPK_FUNIT)
  {
      switch ( ctrlSel )
      {
        case AUDIO_FU_CTRL_MUTE:
          // Audio control mute cur parameter block consists of only one byte - we thus can send it right away
          // There does not exist a range parameter block for microphoneMute
          TU_LOG2("    Get Mute of channel: %u\r\n", channelNum);
          return tud_control_xfer(rhport, p_request, &speakerMute[channelNum], 1);

        case AUDIO_FU_CTRL_VOLUME:
          switch ( p_request->bRequest )
          {
            case AUDIO_CS_REQ_CUR:
              TU_LOG2("    Get Volume of channel: %u\r\n", channelNum);
              return tud_control_xfer(rhport, p_request, &speakerLogVolume[channelNum], sizeof(speakerLogVolume[channelNum]));

            case AUDIO_CS_REQ_RANGE:
              TU_LOG2("    Get Volume range of channel: %u\r\n", channelNum);

              /* The Volume Control is one of the building blocks of a Feature Unit. A Volume Control must support the
                CUR and RANGE(MIN, MAX, RES) attributes. The settings for the CUR, MIN, and MAX attributes can
                range from +127.9961 dB (0x7FFF) down to -127.9961 dB (0x8001) in steps of 1/256 dB or 0.00390625
                dB (0x0001). The settings for the RES attribute can only have positive values and range from 1/256 dB
                (0x0001) to +127.9961 dB (0x7FFF).
                In addition, code 0x8000, representing silence (i.e., -∞ dB), must always be implemented. However, it
                must never be reported as the MIN attribute value. */

              // Copy values - only for testing - better is version below
              audio_control_range_2_n_t(1) ret;

              /* From 1 (0dB) down to 1/65536 (-96dB) */
              ret.wNumSubRanges = 1;
              ret.subrange[0].bMin = -96 * 256;
              ret.subrange[0].bMax = 0;
              ret.subrange[0].bRes = 1;

              return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*) &ret, sizeof(ret));

              // Unknown/Unsupported control
            default:
              TU_BREAKPOINT();
              return false;
          }
        break;

          // Unknown/Unsupported control
        default:
          TU_BREAKPOINT();
          return false;
      }
  }

  // Feature unit
  if (entityID == AUDIO_CTRL_ID_MIC_FUNIT)
  {
    switch ( ctrlSel )
    {
      case AUDIO_FU_CTRL_MUTE:
        // Audio control microphoneMute cur parameter block consists of only one byte - we thus can send it right away
        // There does not exist a range parameter block for microphoneMute
        TU_LOG2("    Get Mute of channel: %u\r\n", channelNum);
        return tud_control_xfer(rhport, p_request, &microphoneMute[channelNum], 1);

      case AUDIO_FU_CTRL_VOLUME:
        switch ( p_request->bRequest )
        {
          case AUDIO_CS_REQ_CUR:
            TU_LOG2("    Get Volume of channel: %u\r\n", channelNum);
            return tud_control_xfer(rhport, p_request, &microphoneLogVolume[channelNum], sizeof(microphoneLogVolume[channelNum]));

          case AUDIO_CS_REQ_RANGE:
            TU_LOG2("    Get Volume range of channel: %u\r\n", channelNum);

            /* The Volume Control is one of the building blocks of a Feature Unit. A Volume Control must support the
              CUR and RANGE(MIN, MAX, RES) attributes. The settings for the CUR, MIN, and MAX attributes can
              range from +127.9961 dB (0x7FFF) down to -127.9961 dB (0x8001) in steps of 1/256 dB or 0.00390625
              dB (0x0001). The settings for the RES attribute can only have positive values and range from 1/256 dB
              (0x0001) to +127.9961 dB (0x7FFF).
              In addition, code 0x8000, representing silence (i.e., -∞ dB), must always be implemented. However, it
              must never be reported as the MIN attribute value. */

            // Copy values - only for testing - better is version below
            audio_control_range_2_n_t(1) ret;

            /* From 1 (0dB) down to 1/65536 (-96dB) */
            ret.wNumSubRanges = 1;
            ret.subrange[0].bMin = -96 * 256;
            ret.subrange[0].bMax = 0;
            ret.subrange[0].bRes = 1;

            return tud_audio_buffer_and_schedule_control_xfer(rhport, p_request, (void*) &ret, sizeof(ret));

            // Unknown/Unsupported control
          default:
            TU_BREAKPOINT();
            return false;
        }
      break;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  // Clock Source unit
  if ( entityID == AUDIO_CTRL_ID_CLOCK )
  {
    switch ( ctrlSel )
    {
      case AUDIO_CS_CTRL_SAM_FREQ:
        // channelNum is always zero in this case
        switch ( p_request->bRequest )
        {
          case AUDIO_CS_REQ_CUR:
            TU_LOG2("    Get Sample Freq.\r\n");
            uint32_t sampFreq = AUDIO_SAMPLE_RATE;
            return tud_control_xfer(rhport, p_request, &sampFreq, sizeof(sampFreq));

          case AUDIO_CS_REQ_RANGE:
            TU_LOG2("    Get Sample Freq. range\r\n");
            audio_control_range_4_n_t(1) sampleFreqRng = {
                .wNumSubRanges = 1,
                .subrange[0] = {
                    .bMin = AUDIO_SAMPLE_RATE,
                    .bMax = AUDIO_SAMPLE_RATE,
                    .bRes = 0
                }
            };

            return tud_control_xfer(rhport, p_request, &sampleFreqRng, sizeof(sampleFreqRng));

           // Unknown/Unsupported control
          default:
            TU_BREAKPOINT();
            return false;
        }
      break;

      case AUDIO_CS_CTRL_CLK_VALID:
        // Only cur attribute exists for this request
        TU_LOG2("    Get Sample Freq. valid\r\n");

        uint8_t clkValid = 1;
        return tud_control_xfer(rhport, p_request, &clkValid, sizeof(clkValid));

      // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  TU_LOG2("  Unsupported entity: %d\r\n", entityID);
  return false;     // Yet not implemented
}

bool tud_audio_tx_done_pre_load_cb(uint8_t rhport, uint8_t itf, uint8_t ep_in, uint8_t cur_alt_setting) {
    (void) rhport;
    (void) itf;
    (void) ep_in;
    (void) cur_alt_setting;

    if (microphoneState == STATE_START) {
        /* Start ADC sampling as soon as device stacks starts loading data (will be a ZLP for first frame) */
        NVIC_EnableIRQ(ADC1_2_IRQn);
        microphoneState = STATE_RUN;
    }

    return true;
}

bool tud_audio_rx_done_post_read_cb(uint8_t rhport, uint16_t n_bytes_received, uint8_t func_id, uint8_t ep_out, uint8_t cur_alt_setting)
{
    /* Get number of total bytes available in FIFO */
    uint16_t count = tud_audio_available();

    /* Calculate min/max/average of buffer fill level */
    if ( (count - n_bytes_received) < speakerBufferLvlMin) speakerBufferLvlMin = count - n_bytes_received;
    if ( count > speakerBufferLvlMax) speakerBufferLvlMax = count;
    speakerBufferLvlAvg = ((uint64_t) speakerBufferLvlAvg * (65536 - SPEAKER_BUFFERLVL_AVG) + ((uint64_t) count << 16) * SPEAKER_BUFFERLVL_AVG) / 65536.0;

    if (speakerState == STATE_START) {
        if (count >= SPEAKER_BUFFERLVL_TARGET) {
            /* Wait until whe are at buffer target fill level, then start DAC output */
            spkState = STATE_RUN;
            NVIC_EnableIRQ(TIM3_IRQn);
        }

        /* Initialize/override min/max/avg during startup buffering */
        speakerBufferLvlAvg = count;
        speakerBufferLvlMin = count;
        speakerBufferLvlMax = count;
    }

    return true;
}


bool tud_audio_set_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request)
{
    (void) rhport;
    (void) p_request;

    uint16_t itf = p_request->wIndex;
    uint16_t alt = p_request->wValue;

    switch(itf) {
    case ITF_NUM_AUDIO_STREAMING_IN:
        if (alt == 1) {
            /* Microphone channel has been activated */
            microphoneState = STATE_START;
        }
        break;

    case ITF_NUM_AUDIO_STREAMING_OUT:
        if (alt == 1) {
            /* Speaker channel has been activated */
            speakerState = STATE_START;
        }
        break;

    default:
        TU_ASSERT(0, false);
        break;
    }

    return true;
}

bool tud_audio_set_itf_close_EP_cb(uint8_t rhport, tusb_control_request_t const *p_request) {
    (void) rhport;
    (void) p_request;

    uint16_t itf = p_request->wIndex;

    switch (itf) {
    case ITF_NUM_AUDIO_STREAMING_IN:
        /* Microphone channel has been stopped */
        NVIC_DisableIRQ(ADC1_2_IRQn);
        microphoneState = STATE_OFF;
        break;

    case ITF_NUM_AUDIO_STREAMING_OUT:
        /* Speaker channel has been stopped */
        NVIC_DisableIRQ(TIM3_IRQn);
        speakerState = STATE_OFF;
        break;

    default:
        TU_ASSERT(0, false);
        break;
    }

    return true;
}

void tud_audio_feedback_params_cb(uint8_t func_id, uint8_t alt_itf, audio_feedback_params_t* feedback_param)
{
    /* Configure parameters for feedback endpoint */
    feedback_param->frequency.mclk_freq = USB_SOF_TIMER_HZ;
    feedback_param->sample_freq = AUDIO_SAMPLE_RATE;
    feedback_param->method = AUDIO_FEEDBACK_METHOD_FREQUENCY_FIXED;
}

TU_ATTR_FAST_FUNC void tud_audio_feedback_interval_isr(uint8_t func_id, uint32_t frame_number, uint8_t interval_shift)
{
    static uint32_t prev_cycles = 0;
    uint32_t this_cycles = USB_SOF_TIMER_CCR; /* Load from capture register, which is set in tu_stm32_sof_cb */
    uint32_t feedback;

    /* Calculate number of master clock cycles between now and last call */
    uint32_t cycles = (uint32_t) (((uint64_t) this_cycles - prev_cycles) & 0xFFFFFFFFUL);
    TU_ASSERT(cycles != 0, /**/);
    /* Prepare for next time */
    prev_cycles = this_cycles;

    /* Calculate the feedback value, taken from tinyusb stack */
    uint64_t fb64 = (((uint64_t) cycles) * AUDIO_SAMPLE_RATE) << 16;
    feedback = (uint32_t) (fb64 / USB_SOF_TIMER_HZ);

    uint32_t min_value = (AUDIO_SAMPLE_RATE/1000 - 1) << 16; /* 1000 for full-speed USB */
    uint32_t max_value = (AUDIO_SAMPLE_RATE/1000 + 1) << 16;

    /* Couple the buffer level bias to the feedback value to avoid buffer drift */
    if (speakerState == STATE_RUN) {
        int32_t bias = (int32_t) speakerBufferLvlAvg - ((int32_t) SPEAKER_BUFFERLVL_TARGET << 16); /* 16.16 format same as feedback */
        feedback -= ((int64_t) bias * SPEAKER_BUFLVL_FB_COUPLING) / 65536;
    }

    /* Limit */
    if ( feedback > max_value ) feedback = max_value;
    if ( feedback < min_value ) feedback = min_value;

    /* Send to host */
    tud_audio_n_fb_set(func_id, feedback);

    /* Handle min/max/avg */
    if (feedback < speakerFeedbackMin) speakerFeedbackMin = feedback;
    if (feedback > speakerFeedbackMax) speakerFeedbackMax = feedback;
    speakerFeedbackAvg = (speakerFeedbackAvg * (65536 - SPEAKER_FEEDBACK_AVG) + ((uint64_t) feedback << 16) * SPEAKER_FEEDBACK_AVG) / 65536.0;

    if (speakerState == STATE_START) {
        /* Initialize/overwrite min/max/avg during start */
        speakerFeedbackAvg = (uint64_t) feedback << 16;
        speakerFeedbackMin = feedback;
        speakerFeedbackMax = feedback;
    }
}

void ADC1_2_IRQHandler (void)
{
    if (ADC2->ISR & ADC_ISR_EOS) {
        ADC2->ISR = ADC_ISR_EOS;
        /* Get ADC sample */
        int16_t sample = ((int32_t) ADC2->DR - 32768) & 0xFFFFU;

        /* Get volume */
        uint16_t volume = !microphoneMute[1] ? microphoneLinVolume[1] : 0;

        /* Scale with 16-bit unsigned volume and round */
        sample = (int16_t) (((int32_t) sample * volume + (sample > 0 ? 32768 : -32768)) / 65536);

        /* Store in FIFO */
        tud_audio_write (&sample, sizeof(sample));
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM3->SR & TIM_SR_UIF) {
        TIM3->SR = (uint32_t) ~TIM_SR_UIF;
        int16_t sample = 0x0000;

        /* Read from FIFO, leave sample at 0 if fifo empty */
        tud_audio_read(&sample, sizeof(sample));

        /* Get volume */
        uint16_t volume = !speakerMute[1] ? speakerLinVolume[1] : 0;

        /* Scale with 16-bit unsigned volume and round */
        sample = (int16_t) (((int32_t) sample * volume + (sample > 0 ? 32768 : -32768)) / 65536);

        /* Load DAC holding register with sample */
        DAC1->DHR12L1 = ((int32_t) sample + 32768) & 0xFFFFU;
    }
}

static void GPIO_Init(void)
{
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef ADCInGpio;
    ADCInGpio.Pin = GPIO_PIN_2;
    ADCInGpio.Mode = GPIO_MODE_ANALOG;
    ADCInGpio.Pull = GPIO_NOPULL;
    ADCInGpio.Speed = GPIO_SPEED_FREQ_LOW;
    ADCInGpio.Alternate = 0;
    HAL_GPIO_Init(GPIOB, &ADCInGpio);

    GPIO_InitTypeDef SamplerateGpio;
    SamplerateGpio.Pin = GPIO_PIN_0;
    SamplerateGpio.Mode = GPIO_MODE_AF_PP;
    SamplerateGpio.Pull = GPIO_NOPULL;
    SamplerateGpio.Speed = GPIO_SPEED_FREQ_HIGH;
    SamplerateGpio.Alternate = GPIO_AF2_TIM3;
    HAL_GPIO_Init(GPIOB, &SamplerateGpio);

    GPIO_InitTypeDef DACOutGpio;
    DACOutGpio.Pin = GPIO_PIN_4;
    DACOutGpio.Mode = GPIO_MODE_ANALOG;
    DACOutGpio.Pull = GPIO_NOPULL;
    DACOutGpio.Speed = GPIO_SPEED_FREQ_LOW;
    DACOutGpio.Alternate = 0;
    HAL_GPIO_Init(GPIOA, &DACOutGpio);
}

static void Timer_Init(void)
{
    __HAL_RCC_TIM3_CLK_ENABLE();
    /* TODO: Use TIM6? */
    /* TIM3_TRGO triggers ADC2 */
    TIM3->CR1 = TIM_CLOCKDIVISION_DIV1 | TIM_COUNTERMODE_UP | TIM_AUTORELOAD_PRELOAD_ENABLE;
    TIM3->CR2 = TIM_TRGO_UPDATE;
    TIM3->PSC = 0;
    TIM3->ARR = (2 * HAL_RCC_GetPCLK1Freq() / AUDIO_SAMPLE_RATE) - 1;
    TIM3->EGR = TIM_EGR_UG;
#if 1 /* Output sample rate on compare channel 3 */
    TIM3->CCMR2 =  TIM_OCMODE_PWM1 | TIM_CCMR2_OC3PE;
    TIM3->CCER = (0 << TIM_CCER_CC3P_Pos) | TIM_CCER_CC3E;
    TIM3->CCR3 = 500;
#endif
    TIM3->DIER = TIM_DIER_UIE;
    TIM3->CR1 |= TIM_CR1_CEN;

    NVIC_SetPriority(TIM3_IRQn, AIOC_IRQ_PRIO_AUDIO);
}

static void ADC_Init(void)
{
    __HAL_RCC_ADC2_CLK_ENABLE();

    ADC2->CR = 0x00 << ADC_CR_ADVREGEN_Pos;
    ADC2->CR = 0x01 << ADC_CR_ADVREGEN_Pos;

    for (uint32_t i=0; i<200; i++) {
        asm volatile ("nop");
    }

    /* Select AHB clock */
    ADC12_COMMON->CCR = (0x1 << ADC12_CCR_CKMODE_Pos) | (0x00 << ADC12_CCR_MULTI_Pos);

    ADC2->CR |= ADC_CR_ADCAL;

    while (ADC2->CR & ADC_CR_ADCAL)
        ;

    ADC2->CR |= ADC_CR_ADEN;

    /* Wait for ADC to be ready */
    while (!(ADC2->ISR & ADC_ISR_ADRDY))
        ;

    /* External Trigger on TIM3_TRGO, left aligned data with 12 bit resolution */
    ADC2->CFGR = (0x01 << ADC_CFGR_EXTEN_Pos)  | (0x04 << ADC_CFGR_EXTSEL_Pos) | (ADC_CFGR_ALIGN) | (0x00 << ADC_CFGR_RES_Pos);

    /* Maximum sample time of 601.5 cycles for channel 12. */
    ADC2->SMPR2 = 0x7 << ADC_SMPR2_SMP12_Pos;

    /* Sample only channel 12 in a regular sequence */
    ADC2->SQR1 = (12 << ADC_SQR1_SQ1_Pos) | (0 << ADC_SQR1_L_Pos);

    /* Enable Interrupt Request */
    ADC2->IER = ADC_IER_EOSIE;

    /* Start ADC */
    ADC2->CR |= ADC_CR_ADSTART;

    NVIC_SetPriority(ADC1_2_IRQn, AIOC_IRQ_PRIO_AUDIO);
}

void DAC_Init(void)
{
    __HAL_RCC_DAC1_CLK_ENABLE();

    /* TSEL1 == TIM3 trigger */
    __HAL_REMAPTRIGGER_ENABLE(HAL_REMAPTRIGGER_DAC1_TRIG);

    DAC->CR = (0x1 << DAC_CR_TSEL1_Pos) | DAC_CR_TEN1 | DAC_CR_EN1;
}

void USB_AudioInit(void)
{
    GPIO_Init();
    Timer_Init();
    ADC_Init();
    DAC_Init();
}

void USB_AudioGetSpeakerFeedbackStats(usb_audio_fbstats_t * status)
{
    *status = (usb_audio_fbstats_t) {
        .feedbackMin = speakerFeedbackMin,
        .feedbackMax = speakerFeedbackMax,
        .feedbackAvg = (uint32_t) (speakerFeedbackAvg >> 16)
    };
}

void USB_AudioGetSpeakerBufferStats(usb_audio_bufstats_t * status)
{
    *status = (usb_audio_bufstats_t) {
        .bufLevelMin = speakerBufferLvlMin,
        .bufLevelMax =  speakerBufferLvlMax,
        .bufLevelAvg = (uint16_t) (speakerBufferLvlAvg >> 16)
    };

}

#include <io.h>
#include "usb_audio.h"
#include "stm32f3xx_hal.h"
#include "aioc.h"
#include "tusb.h"
#include "usb.h"
#include "cos.h"
#include <math.h>

/* The one and only supported sample rate */
#define DEFAULT_SAMPLE_RATE   	48000
/* This is feedback average responsivity with a denominator of 65536 */
#define SPEAKER_FEEDBACK_AVG    32
/* This is buffer level average responsivity with a denominator of 65536 */
#define SPEAKER_BUFFERLVL_AVG   64
/* This is the amount of buffer level to feedback coupling with a denominator of 65536 to prevent buffer drift */
#define SPEAKER_BUFLVL_FB_COUPLING 1
/* We try to stay on this target with the buffer level */
#define SPEAKER_BUFFERLVL_TARGET (5 * CFG_TUD_AUDIO_EP_SZ_OUT) /* Keep our buffer at 5 frames, i.e. 5ms at full-speed USB and maximum sample rate */


typedef enum {
    SAMPLERATE_48000, /* The high-quality default */
    SAMPLERATE_32000, /* For completeness sake, support 32 kHz as well */
    SAMPLERATE_24000, /* Just half of 48 kHz */
    SAMPLERATE_22050, /* For APRSdroid support. NOTE: Has approx. 90 ppm of clock frequency error (ca. 22052 Hz) */
    SAMPLERATE_16000, /* On ARM platforms, direwolf will by default, divide configured sample rate by 3, thus support 16 kHz */
    SAMPLERATE_12000, /* Just a quarter of 48 kHz */
    SAMPLERATE_11025, /* NOTE: Has approx. 90 ppm of clock frequency error (ca. 11026 Hz) */
    SAMPLERATE_8000,
    SAMPLERATE_COUNT /* Has to be last element */
} samplerate_t;

typedef enum {
    STATE_OFF,
    STATE_START,
    STATE_RUN
} state_t;

/* Various state variables. N+1 because 0 is always the master channel */
static bool microphoneMute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1];
static bool speakerMute[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1];
static int16_t microphoneLogVolume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX + 1] = { [0 ... CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_TX] = 0 }; /* in dB */
static int16_t speakerLogVolume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1] = { [0 ... CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX] = 0 }; /* in dB */
static uint16_t microphoneLinVolume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1] = { [0 ... CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX] = 65535 }; /* 0.16 format */
static uint16_t speakerLinVolume[CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX + 1] = { [0 ... CFG_TUD_AUDIO_FUNC_1_N_CHANNELS_RX] = 65535 }; /* 0.16 format */
static uint32_t microphoneSampleFreq = DEFAULT_SAMPLE_RATE; /* Current (requested) sample rate */
static uint32_t speakerSampleFreq = DEFAULT_SAMPLE_RATE; /* Current (requested) sample rate */
static uint64_t speakerFeedbackAvg; /* 32.32 format */
static uint32_t speakerFeedbackMin;
static uint32_t speakerFeedbackMax;
static uint32_t speakerBufferLvlAvg; /* 16.16 format */
static uint16_t speakerBufferLvlMin;
static uint16_t speakerBufferLvlMax;
static volatile uint32_t microphoneSampleFreqCfg; /* Actual configured sample rate in the timer hardware. May be different from requested for odd sample rates */
static volatile uint32_t speakerSampleFreqCfg; /* Actual configured sample rate in the timer hardware. May be different from requested for odd sample rates */
static volatile state_t microphoneState = STATE_OFF;
static volatile state_t speakerState = STATE_OFF;

static audio_control_range_4_n_t(SAMPLERATE_COUNT) sampleFreqRng = {
    .wNumSubRanges = SAMPLERATE_COUNT,
    .subrange = {
        [SAMPLERATE_48000] = {.bMin = 48000, .bMax = 48000, .bRes = 0},
        [SAMPLERATE_32000] = {.bMin = 32000, .bMax = 32000, .bRes = 0},
        [SAMPLERATE_24000] = {.bMin = 24000, .bMax = 24000, .bRes = 0},
        [SAMPLERATE_22050] = {.bMin = 22050, .bMax = 22050, .bRes = 0},
        [SAMPLERATE_16000] = {.bMin = 16000, .bMax = 16000, .bRes = 0},
        [SAMPLERATE_12000] = {.bMin = 12000, .bMax = 12000, .bRes = 0},
        [SAMPLERATE_11025] = {.bMin = 11025, .bMax = 11025, .bRes = 0},
        [SAMPLERATE_8000]  = {.bMin =  8000, .bMax =  8000, .bRes = 0},
    }
};

/* Prototypes of static functions */
static void Timer_ADC_Init(void);
static void Timer_DAC_Init(void);
static void ADC_Init(void);
static void DAC_Init(void);
static void Timeout_Timers_Init(void);

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

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] =  (settingsRegMap[SETTINGS_REG_INFO_AUDIO0] & ~(SETTINGS_REG_INFO_AUDIO0_RECMUTE0_MASK | SETTINGS_REG_INFO_AUDIO0_RECMUTE1_MASK)) \
                                                | (microphoneMute[0] ? SETTINGS_REG_INFO_AUDIO0_RECMUTE0_MASK : 0) \
                                                | (microphoneMute[1] ? SETTINGS_REG_INFO_AUDIO0_RECMUTE1_MASK : 0);

        TU_LOG2("    Set Mute: %d of channel: %u\r\n", microphoneMute[channelNum], channelNum);
      return true;

      case AUDIO_FU_CTRL_VOLUME:
        // Request uses format layout 2
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

        microphoneLogVolume[channelNum] = ((audio_control_cur_2_t*) pBuff)->bCur;
        double logVolume = microphoneLogVolume[channelNum] / 256; /* format is 7.8 fixed point */
        microphoneLinVolume[channelNum] = (microphoneLogVolume[channelNum] != 0x8000) ?
                (uint16_t) (65535 * pow(10, logVolume/20) + 0.5) : 0; /* log to linear with rounding */

        settingsRegMap[SETTINGS_REG_INFO_AUDIO3] = ((((uint32_t) microphoneLinVolume[0]) << SETTINGS_REG_INFO_AUDIO3_RECVOL0_OFFS) & SETTINGS_REG_INFO_AUDIO3_RECVOL0_MASK) \
                                               | ((((uint32_t) microphoneLinVolume[1]) << SETTINGS_REG_INFO_AUDIO3_RECVOL1_OFFS) & SETTINGS_REG_INFO_AUDIO3_RECVOL1_MASK);

        TU_LOG2("    Set Volume: %u.%u dB of channel: %u\r\n", microphoneLogVolume[channelNum] / 256, microphoneLogVolume[channelNum] % 256, channelNum);
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

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] =  (settingsRegMap[SETTINGS_REG_INFO_AUDIO0] & ~(SETTINGS_REG_INFO_AUDIO0_PLAYMUTE0_MASK | SETTINGS_REG_INFO_AUDIO0_PLAYMUTE1_MASK)) \
                                                | (speakerMute[0] ? SETTINGS_REG_INFO_AUDIO0_PLAYMUTE0_MASK : 0) \
                                                | (speakerMute[1] ? SETTINGS_REG_INFO_AUDIO0_PLAYMUTE1_MASK : 0);

        TU_LOG2("    Set Mute: %d of channel: %u\r\n", speakerMute[channelNum], channelNum);

      return true;

      case AUDIO_FU_CTRL_VOLUME:
        // Request uses format layout 2
        TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_2_t));

        speakerLogVolume[channelNum] = ((audio_control_cur_2_t*) pBuff)->bCur;
        double logVolume = (double) speakerLogVolume[channelNum] / 256; /* format is 7.8 fixed point */
        speakerLinVolume[channelNum] = (speakerLogVolume[channelNum] != 0x8000) ?
                (uint16_t) (65535 * pow(10, logVolume/20) + 0.5) : 0; /* log to linear with rounding */

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO9] = ((((uint32_t) speakerLinVolume[0]) << SETTINGS_REG_INFO_AUDIO9_PLAYVOL0_OFFS) & SETTINGS_REG_INFO_AUDIO9_PLAYVOL0_MASK) \
                                               | ((((uint32_t) speakerLinVolume[1]) << SETTINGS_REG_INFO_AUDIO9_PLAYVOL1_OFFS) & SETTINGS_REG_INFO_AUDIO9_PLAYVOL1_MASK);


        TU_LOG2("    Set Volume: %u.%u dB of channel: %u\r\n", microphoneLogVolume[channelNum] / 256, microphoneLogVolume[channelNum] % 256, channelNum);
      return true;

        // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
      return false;
    }
  }

  if ( entityID == AUDIO_CTRL_ID_MIC_CLOCK )
  {
    switch ( ctrlSel )
    {
      case AUDIO_CS_CTRL_SAM_FREQ:
        // channelNum is always zero in this case
        switch ( p_request->bRequest )
        {
          case AUDIO_CS_REQ_CUR:
            TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_4_t));
            microphoneSampleFreq = ((audio_control_cur_4_t*) pBuff)->bCur;
            TU_LOG2("    Set Mic. Sample Freq: %lu\r\n", microphoneSampleFreq);

            Timer_ADC_Init();

            /* Update debug register */
            settingsRegMap[SETTINGS_REG_INFO_AUDIO2] = (((uint32_t) microphoneSampleFreqCfg) << SETTINGS_REG_INFO_AUDIO2_RECRATE_OFFS) & SETTINGS_REG_INFO_AUDIO2_RECRATE_MASK;

            return true;

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

  if ( entityID == AUDIO_CTRL_ID_SPK_CLOCK )
  {
    switch ( ctrlSel )
    {
      case AUDIO_CS_CTRL_SAM_FREQ:
        // channelNum is always zero in this case
        switch ( p_request->bRequest )
        {
          case AUDIO_CS_REQ_CUR:
            TU_VERIFY(p_request->wLength == sizeof(audio_control_cur_4_t));
            speakerSampleFreq = ((audio_control_cur_4_t*) pBuff)->bCur;
            TU_LOG2("    Set Spk. Sample Freq: %lu\r\n", speakerSampleFreq);

            Timer_DAC_Init();

            /* Update debug register */
            settingsRegMap[SETTINGS_REG_INFO_AUDIO8] = (((uint32_t) speakerSampleFreqCfg) << SETTINGS_REG_INFO_AUDIO8_PLAYRATE_OFFS) & SETTINGS_REG_INFO_AUDIO8_PLAYRATE_MASK;

            return true;

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
  if ( entityID == AUDIO_CTRL_ID_MIC_CLOCK )
  {
    switch ( ctrlSel )
    {
      case AUDIO_CS_CTRL_SAM_FREQ:
        // channelNum is always zero in this case
        switch ( p_request->bRequest )
        {
          case AUDIO_CS_REQ_CUR:
            TU_LOG2("    Get Mic. Sample Freq.\r\n");
            return tud_control_xfer(rhport, p_request, &microphoneSampleFreq, sizeof(microphoneSampleFreq));

          case AUDIO_CS_REQ_RANGE:
            TU_LOG2("    Get Mic. Sample Freq. range\r\n");
            return tud_control_xfer(rhport, p_request, &sampleFreqRng, sizeof(sampleFreqRng));

           // Unknown/Unsupported control
          default:
            TU_BREAKPOINT();
            return false;
        }
      break;

      case AUDIO_CS_CTRL_CLK_VALID:
        // Only cur attribute exists for this request
        TU_LOG2("    Get Mic Sample Freq. valid\r\n");

        uint8_t clkValid = 1;
        return tud_control_xfer(rhport, p_request, &clkValid, sizeof(clkValid));

      // Unknown/Unsupported control
      default:
        TU_BREAKPOINT();
        return false;
    }
  }

  // Clock Source unit
  if ( entityID == AUDIO_CTRL_ID_SPK_CLOCK )
  {
    switch ( ctrlSel )
    {
      case AUDIO_CS_CTRL_SAM_FREQ:
        // channelNum is always zero in this case
        switch ( p_request->bRequest )
        {
          case AUDIO_CS_REQ_CUR:
            TU_LOG2("    Get Spk. Sample Freq.\r\n");
            return tud_control_xfer(rhport, p_request, &speakerSampleFreq, sizeof(speakerSampleFreq));

          case AUDIO_CS_REQ_RANGE:
            TU_LOG2("    Get Spk. Sample Freq. range\r\n");
            return tud_control_xfer(rhport, p_request, &sampleFreqRng, sizeof(sampleFreqRng));

           // Unknown/Unsupported control
          default:
            TU_BREAKPOINT();
            return false;
        }
      break;

      case AUDIO_CS_CTRL_CLK_VALID:
        // Only cur attribute exists for this request
        TU_LOG2("    Get Spk. Sample Freq. valid\r\n");

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

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] = (settingsRegMap[SETTINGS_REG_INFO_AUDIO0] & ~SETTINGS_REG_INFO_AUDIO0_RECSTATE_MASK)
                                               | (((uint32_t) SETTINGS_REG_INFO_AUDIO0_RECSTATE_RUN_ENUM) << SETTINGS_REG_INFO_AUDIO0_RECSTATE_OFFS);
    }

    return true;
}

bool tud_audio_rx_done_post_read_cb(uint8_t rhport, uint16_t n_bytes_received, uint8_t func_id, uint8_t ep_out, uint8_t cur_alt_setting)
{
    /* Get number of total bytes available in FIFO */
    uint16_t count = tud_audio_available();

    /* Calculate min/max/average statistics of buffer fill level */
    if ( (count - n_bytes_received) < speakerBufferLvlMin) speakerBufferLvlMin = count - n_bytes_received;
    if ( count > speakerBufferLvlMax) speakerBufferLvlMax = count;
    speakerBufferLvlAvg = ((uint64_t) speakerBufferLvlAvg * (65536 - SPEAKER_BUFFERLVL_AVG) + ((uint64_t) count << 16) * SPEAKER_BUFFERLVL_AVG) / 65536.0;

    if (speakerState == STATE_START) {
        if (count >= SPEAKER_BUFFERLVL_TARGET) {
            /* Wait until whe are at buffer target fill level, then start DAC output */
            speakerState = STATE_RUN;
            NVIC_EnableIRQ(TIM6_DAC1_IRQn);

            /* Update debug register */
            settingsRegMap[SETTINGS_REG_INFO_AUDIO0] = (settingsRegMap[SETTINGS_REG_INFO_AUDIO0] & ~SETTINGS_REG_INFO_AUDIO0_PLAYSTATE_MASK)
                                                   | (((uint32_t) SETTINGS_REG_INFO_AUDIO0_PLAYSTATE_RUN_ENUM) << SETTINGS_REG_INFO_AUDIO0_PLAYSTATE_OFFS);
        }

        /* Initialize/override min/max/avg during startup buffering */
        speakerBufferLvlAvg = count;
        speakerBufferLvlMin = count;
        speakerBufferLvlMax = count;
    }

    /* Write to debug registers */
    settingsRegMap[SETTINGS_REG_INFO_AUDIO10] = ((uint32_t) (speakerBufferLvlAvg >> 16) << SETTINGS_REG_INFO_AUDIO10_PLAYBUFAVG_OFFS) & SETTINGS_REG_INFO_AUDIO10_PLAYBUFAVG_MASK;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO11] = ((uint32_t) speakerBufferLvlMin         << SETTINGS_REG_INFO_AUDIO11_PLAYBUFMIN_OFFS) & SETTINGS_REG_INFO_AUDIO11_PLAYBUFMIN_MASK;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO12] = ((uint32_t) speakerBufferLvlMax         << SETTINGS_REG_INFO_AUDIO12_PLAYBUFMAX_OFFS) & SETTINGS_REG_INFO_AUDIO12_PLAYBUFMAX_MASK;

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

            /* Update VCOS/VPTT timeouts */
            Timeout_Timers_Init();

            /* Update debug register */
            settingsRegMap[SETTINGS_REG_INFO_AUDIO0] = (settingsRegMap[SETTINGS_REG_INFO_AUDIO0] & ~SETTINGS_REG_INFO_AUDIO0_RECSTATE_MASK)
                                                   | (((uint32_t) SETTINGS_REG_INFO_AUDIO0_RECSTATE_START_ENUM) << SETTINGS_REG_INFO_AUDIO0_RECSTATE_OFFS);
        }
        break;

    case ITF_NUM_AUDIO_STREAMING_OUT:
        if (alt == 1) {
            /* Speaker channel has been activated */
            speakerState = STATE_START;

            /* Update VCOS/VPTT timeouts */
            Timeout_Timers_Init();

            /* Update debug register */
            settingsRegMap[SETTINGS_REG_INFO_AUDIO0] = (settingsRegMap[SETTINGS_REG_INFO_AUDIO0] & ~SETTINGS_REG_INFO_AUDIO0_PLAYSTATE_MASK)
                                                   | (((uint32_t) SETTINGS_REG_INFO_AUDIO0_PLAYSTATE_START_ENUM) << SETTINGS_REG_INFO_AUDIO0_PLAYSTATE_OFFS);
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

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] = (settingsRegMap[SETTINGS_REG_INFO_AUDIO0] & ~SETTINGS_REG_INFO_AUDIO0_RECSTATE_MASK)
                                               | (((uint32_t) SETTINGS_REG_INFO_AUDIO0_RECSTATE_OFF_ENUM) << SETTINGS_REG_INFO_AUDIO0_RECSTATE_OFFS);
        break;

    case ITF_NUM_AUDIO_STREAMING_OUT:
        /* Speaker channel has been stopped */
        NVIC_DisableIRQ(TIM6_DAC1_IRQn);
        speakerState = STATE_OFF;

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] = (settingsRegMap[SETTINGS_REG_INFO_AUDIO0] & ~SETTINGS_REG_INFO_AUDIO0_PLAYSTATE_MASK)
                                               | (((uint32_t) SETTINGS_REG_INFO_AUDIO0_PLAYSTATE_OFF_ENUM) << SETTINGS_REG_INFO_AUDIO0_PLAYSTATE_OFFS);
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
    feedback_param->sample_freq = speakerSampleFreqCfg;
    feedback_param->method = AUDIO_FEEDBACK_METHOD_FREQUENCY_FIXED;
}

TU_ATTR_FAST_FUNC void tud_audio_feedback_interval_isr(uint8_t func_id, uint32_t frame_number, uint8_t interval_shift)
{
    static uint32_t prev_cycles = 0;
    uint32_t this_cycles = USB_SOF_TIMER_CNT;
    uint32_t feedback;

    /* Calculate number of master clock cycles between now and last call */
    uint32_t cycles = (uint32_t) (((uint64_t) this_cycles - prev_cycles) & 0xFFFFFFFFUL);
    TU_ASSERT(cycles != 0, /**/);
    /* Prepare for next time */
    prev_cycles = this_cycles;

    /* Calculate the feedback value, taken from tinyusb stack */
    uint64_t fb64 = (((uint64_t) cycles) * speakerSampleFreqCfg) << 16;
    feedback = (uint32_t) (fb64 / USB_SOF_TIMER_HZ);

    /* Couple the buffer level bias to the feedback value to avoid buffer drift */
    if (speakerState == STATE_RUN) {
        int32_t bias = (int32_t) speakerBufferLvlAvg - ((int32_t) SPEAKER_BUFFERLVL_TARGET << 16); /* 16.16 format same as feedback */
        feedback -= ((int64_t) bias * SPEAKER_BUFLVL_FB_COUPLING) / 65536;
    }

    /* The size of isochronous packets created by the device must be within the limits specified in FMT-2.0 section 2.3.1.1.
     * This means that the deviation of actual packet size from nominal size must not exceed +/- one audio slot
     * (audio slot = channel count samples). */
    uint32_t sampleFreq = speakerSampleFreq;
    uint32_t min_value = (sampleFreq/1000 - 1) << 16; /* 1000 for full-speed USB */
    uint32_t max_value = (sampleFreq/1000 + 1) << 16;

    /* Limit */
    if ( feedback > max_value ) feedback = max_value;
    if ( feedback < min_value ) feedback = min_value;

    /* Send to host */
    tud_audio_n_fb_set(func_id, feedback);

    /* Handle min/max/avg statistics */
    if (feedback < speakerFeedbackMin) speakerFeedbackMin = feedback;
    if (feedback > speakerFeedbackMax) speakerFeedbackMax = feedback;
    speakerFeedbackAvg = (speakerFeedbackAvg * (65536 - SPEAKER_FEEDBACK_AVG) + ((uint64_t) feedback << 16) * SPEAKER_FEEDBACK_AVG) / 65536.0;

    if (speakerState == STATE_START) {
        /* Initialize/overwrite min/max/avg during start */
        speakerFeedbackAvg = (uint64_t) feedback << 16;
        speakerFeedbackMin = feedback;
        speakerFeedbackMax = feedback;
    }

    /* Write to debug registers */
    settingsRegMap[SETTINGS_REG_INFO_AUDIO13] = ((uint32_t) (speakerFeedbackAvg >> 16) << SETTINGS_REG_INFO_AUDIO13_PLAYFBAVG_OFFS) & SETTINGS_REG_INFO_AUDIO13_PLAYFBAVG_MASK;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO14] = ((uint32_t) speakerFeedbackMin         << SETTINGS_REG_INFO_AUDIO14_PLAYFBMIN_OFFS) & SETTINGS_REG_INFO_AUDIO14_PLAYFBMIN_MASK;
    settingsRegMap[SETTINGS_REG_INFO_AUDIO15] = ((uint32_t) speakerFeedbackMax         << SETTINGS_REG_INFO_AUDIO15_PLAYFBMAX_OFFS) & SETTINGS_REG_INFO_AUDIO15_PLAYFBMAX_MASK;
}

void ADC1_2_IRQHandler (void)
{
    if (ADC2->ISR & ADC_ISR_EOS) {
        ADC2->ISR = ADC_ISR_EOS;
        /* Get ADC sample */
        int16_t sample = ((int32_t) ADC2->DR - 32768) & 0xFFFFU;

        /* Automatic COS */
        uint16_t cosThreshold = (settingsRegMap[SETTINGS_REG_VCOS_LVLCTRL] & SETTINGS_REG_VCOS_LVLCTRL_THRSHLD_MASK) >> SETTINGS_REG_VCOS_LVLCTRL_THRSHLD_OFFS;

        if (!microphoneMute[1] && ( (sample > cosThreshold) || (sample < -cosThreshold) )) {
            /* Reset timeout and make sure timer is enabled */
            TIM17->EGR = TIM_EGR_UG; /* Generate an update event in the timer */
        }

        /* Get volume */
        uint16_t volume = !microphoneMute[1] ? microphoneLinVolume[1] : 0;

        /* Scale with 16-bit unsigned volume and round */
        sample = (int16_t) (((int32_t) sample * volume + (sample > 0 ? 32768 : -32768)) / 65536);

        /* Store in FIFO */
        tud_audio_write (&sample, sizeof(sample));
    }
}

void TIM6_DAC_IRQHandler(void)
{
    if (TIM6->SR & TIM_SR_UIF) {
        TIM6->SR = (uint32_t) ~TIM_SR_UIF;
        int16_t sample = 0x0000;

        /* Read from FIFO, leave sample at 0 if fifo empty */
        tud_audio_read(&sample, sizeof(sample));

        /* Automatic PTT */
        uint16_t pttThreshold = (settingsRegMap[SETTINGS_REG_VPTT_LVLCTRL] & SETTINGS_REG_VPTT_LVLCTRL_THRSHLD_MASK) >> SETTINGS_REG_VPTT_LVLCTRL_THRSHLD_OFFS;

        if (!speakerMute[1] && ( (sample > pttThreshold) || (sample < -pttThreshold) )) {
            /* Reset timeout and make sure timer is enabled */
            TIM16->EGR = TIM_EGR_UG; /* Generate an update event in the timer */
        }

        /* Get volume */
        uint16_t volume = !speakerMute[1] ? speakerLinVolume[1] : 0;

        /* Scale with 16-bit unsigned volume and round */
        sample = (int16_t) (((int32_t) sample * volume + (sample > 0 ? 32768 : -32768)) / 65536);

        /* Load DAC holding register with sample */
        DAC1->DHR12L1 = ((int32_t) sample + 32768) & 0xFFFFU;
    }
}

void TIM16_IRQHandler(void)
{
    /* This is a timeout counter for the automatic PTT function */
    uint32_t flags = TIM16->SR;

    if (flags & TIM_SR_UIF) {
        /* Timer was reset (via the EGR register). */
        uint32_t cr = TIM16->CR1;
        if (!(cr & TIM_CR1_CEN)) {
            /* If timer was not enabled previously, enable timer and assert PTT */
            TIM16->CR1 = cr | TIM_CR1_CEN;

            /* Update debug register */
            settingsRegMap[SETTINGS_REG_INFO_AUDIO0] |= SETTINGS_REG_INFO_AIOC0_VPTTSTATE_MASK;

            /* Assert enabled PTTs */
            uint8_t pttMask = IO_PTT_MASK_NONE;
            pttMask |= settingsRegMap[SETTINGS_REG_AIOC_IOMUX0] & SETTINGS_REG_AIOC_IOMUX0_OUT1SRC_VPTT_MASK ? IO_PTT_MASK_PTT1 : 0;
            pttMask |= settingsRegMap[SETTINGS_REG_AIOC_IOMUX1] & SETTINGS_REG_AIOC_IOMUX1_OUT2SRC_VPTT_MASK ? IO_PTT_MASK_PTT2 : 0;

            IO_PTTAssert(pttMask);
        }
    } else if (flags & TIM_SR_CC1IF) {
        /* The idle timeout (without any action on the DAC) was reached. Disable timer and deassert PTT */
        TIM16->CR1 &= ~TIM_CR1_CEN;

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] &= ~SETTINGS_REG_INFO_AIOC0_VPTTSTATE_MASK;

        /* Deassert enabled PTTs */
        uint8_t pttMask = IO_PTT_MASK_NONE;
        pttMask |= settingsRegMap[SETTINGS_REG_AIOC_IOMUX0] & SETTINGS_REG_AIOC_IOMUX0_OUT1SRC_VPTT_MASK ? IO_PTT_MASK_PTT1 : 0;
        pttMask |= settingsRegMap[SETTINGS_REG_AIOC_IOMUX1] & SETTINGS_REG_AIOC_IOMUX1_OUT2SRC_VPTT_MASK ? IO_PTT_MASK_PTT2 : 0;

        IO_PTTDeassert(pttMask);
    }

    TIM16->SR = ~flags;
}

void TIM17_IRQHandler(void)
{
    /* This is a timeout counter for the automatic COS function */
    uint32_t flags = TIM17->SR;

    if (flags & TIM_SR_UIF) {
        /* Timer was reset (via the EGR register). */
        uint32_t cr = TIM17->CR1;
        if (!(cr & TIM_CR1_CEN)) {
            /* If timer was not enabled previously, enable timer and notify host of COS */
            TIM17->CR1 = cr | TIM_CR1_CEN;

            /* Update debug register */
            settingsRegMap[SETTINGS_REG_INFO_AUDIO0] |= SETTINGS_REG_INFO_AIOC0_VCOSSTATE_MASK;

            /* Set COS state */
            COS_VirtualSetState(0x01);
        }
    } else if (flags & TIM_SR_CC1IF) {
        /* The idle timeout (without any action on the ADC) was reached. Disable timer and notify host */
        TIM17->CR1 &= ~TIM_CR1_CEN;

        /* Update debug register */
        settingsRegMap[SETTINGS_REG_INFO_AUDIO0] &= ~SETTINGS_REG_INFO_AIOC0_VCOSSTATE_MASK;

        /* Set COS state */
        COS_VirtualSetState(0x00);
    }

    TIM17->SR = ~flags;
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

static void Timer_ADC_Init(void)
{
	/* Calculate clock rate divider for requested sample rate with rounding */
	uint32_t timerFreq = (HAL_RCC_GetHCLKFreq() == HAL_RCC_GetPCLK1Freq()) ? HAL_RCC_GetPCLK1Freq() : 2 * HAL_RCC_GetPCLK1Freq();
	uint32_t rateDivider = (timerFreq + microphoneSampleFreq / 2) / microphoneSampleFreq;

	/* Store actually realized samplerate */
	microphoneSampleFreqCfg = timerFreq / rateDivider;

	/* Enable clock and (re-) initialize timer */
    __HAL_RCC_TIM3_CLK_ENABLE();

    /* TIM3_TRGO triggers ADC2 */
    TIM3->CR1 &= ~TIM_CR1_CEN;
    TIM3->CR1 = TIM_CLOCKDIVISION_DIV1 | TIM_COUNTERMODE_UP | TIM_AUTORELOAD_PRELOAD_ENABLE;
    TIM3->CR2 = TIM_TRGO_UPDATE;
    TIM3->PSC = 0;
    TIM3->ARR = rateDivider - 1;
    TIM3->EGR = TIM_EGR_UG;
#if 1 /* Output sample rate on compare channel 3 */
    TIM3->CCMR2 =  TIM_OCMODE_PWM1 | TIM_CCMR2_OC3PE;
    TIM3->CCER = (0 << TIM_CCER_CC3P_Pos) | TIM_CCER_CC3E;
    TIM3->CCR3 = rateDivider/2 - 1;
#endif
    TIM3->CR1 |= TIM_CR1_CEN;
}

static void Timer_DAC_Init(void)
{
    /* Calculate clock rate divider for requested sample rate with rounding */
    uint32_t timerFreq = (HAL_RCC_GetHCLKFreq() == HAL_RCC_GetPCLK1Freq()) ? HAL_RCC_GetPCLK1Freq() : 2 * HAL_RCC_GetPCLK1Freq();
    uint32_t rateDivider = (timerFreq + speakerSampleFreq / 2) / speakerSampleFreq;

    /* Store actually realized samplerate for feedback algorithm to use */
    speakerSampleFreqCfg = timerFreq / rateDivider;

    /* Enable clock and (re-) initialize timer */
    __HAL_RCC_TIM6_CLK_ENABLE();

    /* TIM6_TRGO triggers DAC */
    TIM6->CR1 &= ~TIM_CR1_CEN;
    TIM6->CR1 = TIM_CLOCKDIVISION_DIV1 | TIM_COUNTERMODE_UP | TIM_AUTORELOAD_PRELOAD_ENABLE;
    TIM6->CR2 = TIM_TRGO_UPDATE;
    TIM6->PSC = 0;
    TIM6->ARR = rateDivider - 1;
    TIM6->EGR = TIM_EGR_UG;

    TIM6->DIER = TIM_DIER_UIE;
    TIM6->CR1 |= TIM_CR1_CEN;

    NVIC_SetPriority(TIM6_DAC1_IRQn, AIOC_IRQ_PRIO_AUDIO);
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

static void DAC_Init(void)
{
    __HAL_RCC_DAC1_CLK_ENABLE();

    /* Select TIM6 TRGO as trigger and enable DAC */
    DAC->CR = (0x0 << DAC_CR_TSEL1_Pos) | DAC_CR_TEN1 | DAC_CR_EN1;
}

static void Timeout_Timers_Init()
{
    uint32_t timerFreq = (HAL_RCC_GetHCLKFreq() == HAL_RCC_GetPCLK2Freq()) ? HAL_RCC_GetPCLK2Freq() : 2 * HAL_RCC_GetPCLK2Freq();
    uint32_t pttTimeout = (settingsRegMap[SETTINGS_REG_VPTT_TIMCTRL] & SETTINGS_REG_VPTT_TIMCTRL_TIMEOUT_MASK) >> SETTINGS_REG_VPTT_TIMCTRL_TIMEOUT_OFFS;
    uint32_t cosTimeout = (settingsRegMap[SETTINGS_REG_VCOS_TIMCTRL] & SETTINGS_REG_VCOS_TIMCTRL_TIMEOUT_MASK) >> SETTINGS_REG_VCOS_TIMCTRL_TIMEOUT_OFFS;

    __HAL_RCC_TIM16_CLK_ENABLE();
    __HAL_RCC_TIM17_CLK_ENABLE();

   /* TIM16 and TIM17 are timeout-counters for PTT and COS */
   TIM16->CR1 = TIM_CLOCKDIVISION_DIV1 | TIM_COUNTERMODE_UP;
   TIM16->PSC = timerFreq / 16000 - 1; /* 16 kHz counter */
   TIM16->CCR1 = pttTimeout - 1;
   TIM16->DIER = TIM_DIER_UIE | TIM_DIER_CC1IE;

   TIM17->CR1 = TIM_CLOCKDIVISION_DIV1 | TIM_COUNTERMODE_UP;
   TIM17->PSC = timerFreq / 16000 - 1; /* 16 kHz counter */
   TIM17->CCR1 = cosTimeout - 1;
   TIM17->DIER = TIM_DIER_UIE | TIM_DIER_CC1IE;

   NVIC_SetPriority(TIM16_IRQn, AIOC_IRQ_PRIO_AUDIO);
   NVIC_EnableIRQ(TIM16_IRQn);

   NVIC_SetPriority(TIM17_IRQn, AIOC_IRQ_PRIO_AUDIO);
   NVIC_EnableIRQ(TIM17_IRQn);
}

void USB_AudioInit(void)
{
    GPIO_Init();
    Timer_ADC_Init();
    Timer_DAC_Init();
    ADC_Init();
    DAC_Init();

    Timeout_Timers_Init();
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

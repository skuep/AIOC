#include <stdint.h>

#define WPM 20 /* words per minute rate for sending morse */

/* This calculates how many samples for a dit at 48000 samples/sec and 20 WPM */
#define DIT_CYCLES (1.2 / WPM)

/* Morse code has five elements (https://en.wikipedia.org/wiki/Morse_code):
 * * short mark (dit)
 * * long mark (dah)
 * * inter-element gap (between dits and dahs)
 * * short gap (between letters)
 * * medium gap (between words)
 * These are the multipliers for each */
#define SHORT 1
#define LONG 3
#define GAP 1
#define LETTER_GAP 3
#define WORD_GAP 7

void set_timings(char *msg, uint8_t timings[], uint8_t *timings_length);

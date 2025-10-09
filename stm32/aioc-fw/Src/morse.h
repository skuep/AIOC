#include <stdint.h>

/* Length of 1 morse unit (dit) in seconds at 1 WPM */
#define MORSE_UNIT_LENGTH 1.2

/* Morse code has five elements (https://en.wikipedia.org/wiki/Morse_code):
 * * Delay on message start
 * * short mark (dit)
 * * long mark (dah)
 * * inter-element gap (between dits and dahs)
 * * short gap (between letters)
 * * medium gap (between words)
 * These are the multipliers for each */
#define MORSE_HEAD 10
#define MORSE_SHORT 1
#define MORSE_LONG 3
#define MORSE_ELEMENT_GAP 1
#define MORSE_LETTER_GAP 3
#define MORSE_WORD_GAP 7

uint16_t Morse_GenerateTimings(char * messageBuffer, uint8_t messageBufferSize, uint8_t * timingsLUT, uint16_t timingsLUTSize);

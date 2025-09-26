#include "morse.h"
#include "settings.h"

/* convenience macro for calling add_timing() */
#define ADD_TIMING(X) timingsIndex = (timingsIndex < timingsLUTSize) ? \
        timingsIndex + addTiming(timingsLUT, timingsIndex, (X)) : 0

static uint8_t addTiming(uint8_t * timingsLUT, uint16_t timingsIndex, uint8_t value) {
	/* prevent the timings array from overflowing */
    timingsLUT[timingsIndex] = value;
	return 1;
}

uint16_t Morse_GenerateTimings(char * messageBuffer, uint8_t messageBufferSize, uint8_t * timingsLUT, uint16_t timingsLUTSize)
{
	char letter;

	uint16_t timingsIndex = 0;

	ADD_TIMING(MORSE_HEAD); /* Give the PTT some time to work */

	/* Go through the message letter by letter and set the appropriate timings */
	for (uint8_t msg_i = 0; ((msg_i < messageBufferSize) && (messageBuffer[msg_i] != '\0')); msg_i++) {
		letter = messageBuffer[msg_i];

		/* if it's lower case, make it upper case */
		if ((letter >= 'a') && (letter <= 'z')) {
			letter ^= ' ';
		}

		switch (letter) {
		case 'A':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'B':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'C':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'D':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'E':
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'F':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'G':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'H':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'I':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'J':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'K':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'L':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'M':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'N':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'O':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'P':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'Q':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'R':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'S':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case 'T':
			ADD_TIMING(MORSE_LONG);
			break;
		case 'U':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'V':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'W':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'X':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'Y':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case 'Z':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
			uint8_t dits = letter - '0';
			for (uint8_t i = 0; i < 5; i++) {
				if (dits > 0) {
					ADD_TIMING(MORSE_SHORT);
					dits--;
				} else {
					ADD_TIMING(MORSE_LONG);
				}
				if (i != 4) {
					ADD_TIMING(MORSE_ELEMENT_GAP);
				}
			}
			break;
		case '6':
		case '7':
		case '8':
		case '9':
			uint8_t dahs = letter - '0' - 5;
			for (uint8_t i = 0; i < 5; i++) {
				if (dahs > 0) {
					ADD_TIMING(MORSE_LONG);
					dahs--;
				} else {
					ADD_TIMING(MORSE_SHORT);
				}
				if (i != 4) {
					ADD_TIMING(MORSE_ELEMENT_GAP);
				}
			}
			break;
		case ' ':
			ADD_TIMING(MORSE_WORD_GAP);
			break;
		case '.':
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		case '-':
			ADD_TIMING(MORSE_LONG);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_SHORT);
			ADD_TIMING(MORSE_ELEMENT_GAP);
			ADD_TIMING(MORSE_LONG);
			break;
		}

		/* If this letter isn't a space, the next letter isn't a space, and we're not
         * at the end: add the LETTER_GAP */
		if ((letter != ' ') && (messageBuffer[msg_i + 1] != ' ') && (messageBuffer[msg_i + 1] != '\0')) {
			ADD_TIMING(MORSE_LETTER_GAP);
		}
	}

	return timingsIndex;
}

#include "morse.h"
#include "fox_hunt.h"
#include "settings.h"

/* convenience macro for calling add_timing() */
#define ADD_TIMING(X) add_timing(timings, timings_length, (X))

void add_timing(uint8_t timings[], uint8_t *timings_length, uint8_t value) {
	/* prevent the timings array from overflowing */
	if (*timings_length >= MAX_TIMINGS) {
		return;
	}
	timings[(*timings_length)++] = value;
}

void set_timings(char *msg, uint8_t timings[], uint8_t *timings_length) {
	char letter;

	*timings_length = 0;

	ADD_TIMING(WORD_GAP); /* Give the PTT some time to work */

	/* Go through the message letter by letter and set the appropriate timings */
	for (uint8_t msg_i = 0; ((msg_i < MAX_ID) && (msg[msg_i] != '\0')); msg_i++) {
		letter = msg[msg_i];

		/* if it's lower case, make it upper case */
		if ((letter >= 'a') && (letter <= 'z')) {
			letter ^= ' ';
		}

		switch (letter) {
		case 'A':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'B':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'C':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'D':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'E':
			ADD_TIMING(SHORT);
			break;
		case 'F':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'G':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'H':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'I':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'J':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'K':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'L':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'M':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'N':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'O':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'P':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'Q':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'R':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'S':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			break;
		case 'T':
			ADD_TIMING(LONG);
			break;
		case 'U':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'V':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'W':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'X':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'Y':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case 'Z':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
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
					ADD_TIMING(SHORT);
					dits--;
				} else {
					ADD_TIMING(LONG);
				}
				if (i != 4) {
					ADD_TIMING(GAP);
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
					ADD_TIMING(LONG);
					dahs--;
				} else {
					ADD_TIMING(SHORT);
				}
				if (i != 4) {
					ADD_TIMING(GAP);
				}
			}
			break;
		case ' ':
			ADD_TIMING(WORD_GAP);
			break;
		case '.':
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		case '-':
			ADD_TIMING(LONG);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(SHORT);
			ADD_TIMING(GAP);
			ADD_TIMING(LONG);
			break;
		}

		/* If this letter isn't a space, the next letter isn't a space, and we're not at the end: add the LETTER_GAP */
		if ((letter != ' ') && (msg[msg_i + 1] != ' ') && (msg[msg_i + 1] != '\0')) {
			ADD_TIMING(LETTER_GAP);
		}
	}
}

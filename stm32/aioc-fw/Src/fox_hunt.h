#define MAX_ID      16  /* Maximum amount of characters in the ID */
#define MAX_TIMINGS 128 /* The size of our timings array (should be good for 16 bytes of ID) */

void fox_hunt_init(void);
void fox_hunt_task(void);
uint8_t fox_hunt_output(void);

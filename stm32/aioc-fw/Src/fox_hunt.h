#ifndef FOX_HUNT_H_
#define FOX_HUNT_H_

#define FOXHUNT_MAX_CHARS       16  /* Maximum amount of characters in the ID */
#define FOXHUNT_MAX_TIMINGS     (FOXHUNT_MAX_CHARS * 8) /* The size of our timings array (should be good for 16 bytes of ID) */

void FoxHunt_Init(void);
void FoxHunt_Tick(void);
uint8_t fox_hunt_output(void);

#endif /* FOX_HUNT_H_ */

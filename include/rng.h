#include <stdint.h>

void initTRNG();
void turnOnTRNG();
void turnOffTRNG();
void getRndNum(uint32_t *rndNum);
volatile uint32_t interPosRndNum(uint32_t min, uint32_t max, uint32_t * buffer);
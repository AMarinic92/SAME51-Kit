#include <stdint.h>

void initTRNG();
void turnOnTRNG();
void turnOffTRNG();
uint32_t getRndNum();
uint32_t interPosRndNum(uint32_t min, uint32_t max);
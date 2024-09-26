#include "trng.h"
#include <same51j20a.h>

void initTRNG(){
    //Turn on the MCLK APB mask for the TRNG (registerC)
    MCLK_REGS->MCLK_APBCMASK |= MCLK_APBCMASK_TRNG_Msk;

}

void turnOnTRNG(){

    initTRNG();
    TRNG_REGS->TRNG_CTRLA |= TRNG_CTRLA_ENABLE_Msk;

}

void turnOffTRNG(){

    TRNG_REGS->TRNG_CTRLA &= ~(TRNG_CTRLA_ENABLE_Msk);
    MCLK_REGS->MCLK_APBCMASK &= ~(MCLK_APBCMASK_TRNG_Msk);

}

uint32_t getRndNum(){

    return TRNG_REGS->TRNG_DATA;
}
uint32_t interPosRndNum(uint32_t min, uint32_t max){
    uint32_t rand = getRndNum();
    float adjust = (float)(rand)/(float)((min-max)/0xFFFF);
    return min+(adjust);

}
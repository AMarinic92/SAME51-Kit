#include "rng.h"
#include <same51j20a.h>

void initTRNG(){
    
    
    //Turn on the MCLK APB mask for the TRNG (registerC)
    MCLK_REGS->MCLK_APBCMASK |= MCLK_APBCMASK_TRNG_Msk;


}

void turnOnTRNG(){

    initTRNG();
    MCLK_REGS->MCLK_APBCMASK |= MCLK_APBCMASK_TRNG_Msk;
    TRNG_REGS->TRNG_CTRLA |= TRNG_CTRLA_ENABLE_Msk;

}

void turnOffTRNG(){

    TRNG_REGS->TRNG_CTRLA &= ~(TRNG_CTRLA_ENABLE_Msk);
    MCLK_REGS->MCLK_APBCMASK &= ~(MCLK_APBCMASK_TRNG_Msk);

}

void getRndNum(uint32_t* rndNum){
    uint32_t rand = 0;
    while((TRNG_REGS->TRNG_INTFLAG & TRNG_INTFLAG_DATARDY_Msk) == 0)
    {
    }
  
    rand = TRNG_REGS->TRNG_DATA;
    rndNum[0] = rand;
}
volatile uint32_t interPosRndNum(uint32_t min, uint32_t max, uint32_t * buffer){
    uint32_t rand = 0;
    getRndNum(&rand);
    uint32_t adjust = (rand)/((min-max)/UINT32_MAX);
    *buffer = (uint32_t)(min+(adjust));
    return (uint32_t)(min+(adjust));

}
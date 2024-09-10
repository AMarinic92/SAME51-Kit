#include "led.h"
#include "sam.h"
#include "same51j20a.h"

void clkLED(){


}

void portLED(char pad, int num){
    int group = 1;
    if( (pad == 'a') || (pad == 'A')){
        group = 0;
    }


    PORT_REGS->GROUP[group].PORT_PINCFG[num] |= PORT_PINCFG_PMUXEN_Msk;
        if(num%2== 0){
            PORT_REGS->GROU[group].PORT_PMUX[num/2] |= PORT_PMUX_PMUXE_F;
        }
        else{
            PORT_REGS->GROU[group].PORT_PMUX[num/2] |= PORT_PMUX_PMUXO_F;
        }

}
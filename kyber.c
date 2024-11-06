#include "sam.h"
#include "button.h"
#include "dcc_stdio.h"
#include "heart.h"
#include "rng.h"
#include "led.h"
#include <stdbool.h>
#include <float.h>
#include <stdio.h>
#include "same51j20a.h"

#define DEBUG_WAIT 10000000UL
// setup our heartbeat to be 1ms: we overflow at 1ms intervals with a 120MHz
// clock uses the SysTicks unit so that we get reliable debugging (timer stops
// on breakpoints)
//  number of millisecond between LED flashes
#define LED_FLASH_MS 1000UL
#define MS_PER_SECOND 1000UL
#define MS_PER_MIN (MS_PER_SECOND * 60UL)
uint32_t interpolateNum(uint32_t min, uint32_t max, uint32_t number);
// NOTE: this overflows every ~50 days, so I'm not going to care here...
// volatile uint32_t msCount = 0;


void initAllPorts()
{
    // LED output
    
    PORT_REGS->GROUP[0].PORT_DIRSET = PORT_PA14;
    PORT_REGS->GROUP[1].PORT_DIRSET = PORT_PB06;
    PORT_REGS->GROUP[1].PORT_DIRSET = PORT_PB07;
    // PORT_REGS->GROUP[1].PORT_OUTTGL = PORT_PB06;
    PORT_REGS->GROUP[0].PORT_OUTSET = PORT_PA14;
    portLED();
} // initAllPorts

void initAllClks()
{
    clkButton();
    clkLED();

} // initAllClks

void initAll()
{
    heartInit();
    initAllPorts();
    initAllClks();
    initButton();
}

// ISR for  external interrupt 15, add processing code as required...
void EIC_EXTINT_15_Handler()
{
    // PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14;
    //  clear the interrupt! and go to the next operating mode
    EIC_REGS->EIC_INTFLAG |= EXTINT15_MASK;
}

uint32_t interpolateNum(uint32_t min, uint32_t max, uint32_t number)
{
    
    uint32_t out = number;

    if (out > max)
    {
        while (out > max)
        {
            out = out >> 1;
        }
        if (out < min)
        {
            out = min;
        }
    }
    else if (out < min)
    {
        while (out < min)
        {
            out = out << 1;
        }
        if (out > max)
        {
            out = max;
        }
    }

    return out;
}

int main(void)
{
#ifndef NDEBUG
    for (int i = 0; i < DEBUG_WAIT; i++)
        ;
#endif

    // enable cache
    // tradeoff: +: really helps with repeated code/data (like when doing
    // animations in a game)
    //           -: results in non-deterministic run-times
    //           +: there *is* a way to lock lines of cache to keep hard
    //           deadline code/data pinned in the cache
    if ((CMCC_REGS->CMCC_SR & CMCC_SR_CSTS_Msk) == 0)
        CMCC_REGS->CMCC_CTRL = CMCC_CTRL_CEN_Msk;

    // sleep to idle (wake on interrupts)
    PM_REGS->PM_SLEEPCFG |= PM_SLEEPCFG_SLEEPMODE_IDLE;

    initAll();
    turnOnTRNG();
    // we want interrupts!
    __enable_irq();

    // some example logging calls
#ifndef NDEBUG
    dbg_write_str("~~~D1EBUG ENABLED~~~\n");
#endif

   
    while (1)
    {
        __WFI();
        if ((get_ticks()% MS_PER_SECOND) == 0)
        {
            PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14;

        }
    }
    return 0;
}

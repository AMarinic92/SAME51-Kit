#include <sam.h>
#include "button.h"
#include "dcc_stdio.h"
#include "heart.h"
#include "rng.h"
#include <stdbool.h>
#include <float.h>
#include <stdio.h>

#define DEBUG_WAIT 10000000UL
// setup our heartbeat to be 1ms: we overflow at 1ms intervals with a 120MHz
// clock uses the SysTicks unit so that we get reliable debugging (timer stops
// on breakpoints)
//  number of millisecond between LED flashes
#define LED_FLASH_MS 1000UL
#define MS_PER_SECOND 1000UL
#define MS_PER_MIN (MS_PER_SECOND * 60UL)
#define MS_MIN_START (MS_PER_SECOND * 25UL)
#define MS_MAX_START (MS_PER_SECOND * 60UL)
#define MS_SLAM_LONG 500UL
#define MS_SLAM_SHORT 250UL
#define MS_SLOW_UP 2UL
#define MS_SLOW_ELAPSE ((MS_PER_SECOND * 150UL)) // for dead code right now
#define MS_SLOW_WAIT (MS_SLOW_UP * 200UL)
#define MAX_DROP_MS (MS_PER_SECOND * 15UL)
#define MIN_DROP_MS (MS_PER_SECOND * 5UL)
#define ACT_STATES 3
#define SLAM_MAX 5UL
#define RND_MAX_MSK 0X2FFF

uint32_t interpolateNum(uint32_t min, uint32_t max, uint32_t number);
// NOTE: this overflows every ~50 days, so I'm not going to care here...
// volatile uint32_t msCount = 0;
volatile uint32_t secCount = 0;

volatile uint32_t actTimer = 0;
uint32_t act_index = 0;
volatile bool is_up = false;
volatile bool is_down = false;
uint32_t randomNumber = 0;
volatile uint32_t lastActTime = 0;
void act_off();
void act_up();
void act_down();
void act_up_time(uint32_t time);
void act_violent();
void act_reset();
void act_quick_up();
void act_slow_up();
void act_random_drop();
void (*actProgs[ACT_STATES])() = {&act_random_drop, &act_quick_up, &act_violent};

void (*actuator)() = &act_reset;

void act_off()
{
    actTimer = 0;
}

void act_violent()
{
    int count = 0;
    int timeMod = 0;
    actTimer = get_ticks() + MS_PER_SECOND;

    // toggle Normally Open relay for UP actuator
    act_up();
    while (get_ticks() < actTimer)
    {
        // wait
    }

    while (count != SLAM_MAX)
    {

        if (count % 2 == 0)
        {
            timeMod = MS_SLAM_LONG;
        }
        else
        {
            timeMod = MS_SLAM_SHORT;
        }
        // Toggle DOWN and UP for half a second (DOWN)
        act_up();
        act_down();

        actTimer = get_ticks() + (timeMod);

        while (get_ticks() < actTimer)
        {
            // wait
        }
        // Toggle DOWN and UP for half a second (UP)
        act_down();
        act_up();

        actTimer = get_ticks() + (timeMod);

        while (get_ticks() < actTimer)
        {
            // wait
        }
        count++;
    }

    act_up();

    actuator = &act_reset;
}

void act_random_drop()
{
    getRndNum(&randomNumber);
    act_up();

    actTimer = get_ticks() + interpolateNum(MIN_DROP_MS, MAX_DROP_MS, randomNumber);
    while (get_ticks() < actTimer)
    {
        /* wait */
    }
    act_up();
    actuator = &act_reset;
}

// I currently don't want both to run at once
// toggles the actuator up
void act_up()
{

    if (!is_down)
    {
        // toggle Normally Open relay for UP actuator
        PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14;
        PORT_REGS->GROUP[1].PORT_OUTTGL = PORT_PB06;
        is_up = !is_up;
    }
}

// toggles the actuator down
void act_down()
{

    if (!is_up)
    {
        PORT_REGS->GROUP[0].PORT_OUTSET = PORT_PA14;
        PORT_REGS->GROUP[1].PORT_OUTTGL = PORT_PB07;
        is_down = !is_down;
    }
}
void act_up_time(uint32_t time)
{

    volatile uint32_t elapse = get_ticks() + time;

    act_up();
    while (get_ticks() < elapse)
    {
        // wait
    }
    // toggle act up to off
    act_up();
}

void act_slow_up()
{
    volatile uint32_t nextWait = 0;
    volatile uint32_t nextUp = 0;
    actTimer = get_ticks() + MS_SLOW_ELAPSE;
    while (get_ticks() < actTimer)
    {
        nextUp = get_ticks() + MS_SLOW_UP;
        act_up();
        while (get_ticks() < nextUp)
        {
            /* code */
        }
        nextWait = get_ticks() + MS_SLOW_WAIT;
        while (get_ticks() < nextWait)
        {
        }
    }
    while (get_ticks() < actTimer)
    {
        nextUp = get_ticks() + MS_SLOW_UP;
        act_up();
        while (get_ticks() < nextUp)
        {
            /* code */
        }
        nextWait = get_ticks() + MS_SLOW_WAIT;
        while (get_ticks() < nextWait)
        {
        }
    }

    actuator = &act_reset;
}

void act_quick_up()
{
    actTimer = get_ticks() + MS_PER_SECOND;
    act_up();
    while (get_ticks() < actTimer)
    {
        // wait
    }
    act_up();

    actuator = &act_reset;
}

void act_reset()
{

    actTimer = get_ticks() + MS_PER_SECOND;

    // toggle Normally Open relay for DOWN actuator
    act_down();
    while (get_ticks() < actTimer)
    {
        // wait
    }
    act_down();
    actuator = &act_off;
}

void initAllPorts()
{
    // LED output
    PORT_REGS->GROUP[0].PORT_DIRSET = PORT_PA14;
    PORT_REGS->GROUP[1].PORT_DIRSET = PORT_PB06;
    PORT_REGS->GROUP[1].PORT_DIRSET = PORT_PB07;
    // PORT_REGS->GROUP[1].PORT_OUTTGL = PORT_PB06;
    PORT_REGS->GROUP[0].PORT_OUTSET = PORT_PA14;

} // initAllPorts

void initAllClks()
{
    clkButton();

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

    // NOTE: the silkscreen on the curiosity board is WRONG! it's PB4 and PB5
    // NOT PA4 and PA5

    // see the header files within include/component for register definitions,
    // which align with the data sheet for the processor e.g. port.h contains
    // the masks and definitions for manipulating gpio

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

    // PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14;
    // Relay ports

    // sleep until we have an interrupt

    // randomNumber = randomNumber&0x7;
    // randomNumber = randomNumber&0x3F;
    uint32_t nextTime = MS_PER_SECOND * 15;

    while (1)
    {
        __WFI();
        actuator();
        //  if ((get_ticks()%(LED_FLASH_MS)) == 0) {
        //     getRndNum(&randomNumber);

        // }

        if ((get_ticks() >= nextTime))
        {

            getRndNum(&act_index);
            // #ifndef NDEBUG

            //     dbg_write_str("Random ACT Number");
            //     dbg_write_u32(&act_index,1);
            // #endif
            if (act_index < 0x7FFFFFFFUL)
            {
                act_index = 1;
            }
            else if (act_index < 0XD5555554UL)
            {
                act_index = 0;
            }
            else
            {
                act_index = 2;
            }

            actuator = actProgs[act_index];
            getRndNum(&randomNumber);
            randomNumber = interpolateNum(MS_MIN_START, MS_MAX_START, randomNumber);
            // #ifndef NDEBUG
            //     dbg_write_str("Random Number");
            //     dbg_write_u32(&randomNumber,1);
            // #endif

            nextTime = get_ticks() + randomNumber;
#ifndef NDEBUG
            uint32_t time = get_ticks();
            // char words[256];
            // sprintf(words, "The value of act after is %d", (int)act_index);
            dbg_write_str("Actuator mode");
            dbg_write_u32(&act_index, 1);
            dbg_write_str(" Curr time ");
            dbg_write_u32(&time, 1);
            dbg_write_str(" Next time ");
            dbg_write_u32(&nextTime, 1);
            dbg_write_str("\n");
#endif
        }
    }
    return 0;
}

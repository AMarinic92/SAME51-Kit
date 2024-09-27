#include <sam.h>
#include "button.h"
#include "dcc_stdio.h"
#include "heart.h"
#include "rng.h"
#include <stdbool.h>


#define DEBUG_WAIT 10000000UL
// setup our heartbeat to be 1ms: we overflow at 1ms intervals with a 120MHz
// clock uses the SysTicks unit so that we get reliable debugging (timer stops
// on breakpoints)
//  number of millisecond between LED flashes
#define LED_FLASH_MS  1000UL
#define MS_PER_SECOND 1000UL
#define MS_SLAM_LONG 500UL
#define MS_SLAM_SHORT 250UL
#define MS_SLOW_UP 2UL
#define MS_SLOW_ELSAPSE (START_MS/2)
#define MS_SLOW_WAIT (MS_SLOW_UP*200)
#define START_MS      (MS_PER_SECOND*300)
#define MAX_DROP_MS MS_PER_SECOND*60
#define MIN_DROP_MS MS_PER_SECOND*5
#define SLAM_MIN 3
#define SLAM_MAX 5
#define ACT_STATES 3


uint32_t interpolateNum(uint32_t min, uint32_t max, uint32_t number);
// NOTE: this overflows every ~50 days, so I'm not going to care here...
// volatile uint32_t msCount = 0;
volatile uint32_t secCount = 0;

volatile uint32_t actTimer = 0;
volatile uint8_t act_index = 0;
volatile bool is_up = false;
volatile bool is_down = false;
volatile uint32_t randomNumber = 0;
void act_off();
void act_up();
void act_down();
void act_up_time(uint32_t time);
void act_violent();
void act_reset();
void act_quick_up();
void act_slow_up();
void act_random_drop();
void (*actProgs[ACT_STATES])() = {&act_random_drop,&act_quick_up,&act_violent};

void (*actuator)() = &act_reset;

void act_off(){
    actTimer = 0;

}

void act_violent(){
    int count = 0;
    int timeMod = 0;
    actTimer = get_ticks()+MS_PER_SECOND;

    //toggle Normally Open relay for UP actuator 
    act_up();
    while(get_ticks()<actTimer){
        //wait
    }
    
    while(count != SLAM_MAX){

        if(count%2 ==0){
            timeMod = MS_SLAM_LONG;
        }
        else{
            timeMod = MS_SLAM_SHORT;
        }
        //Toggle DOWN and UP for half a second (DOWN)
        act_up();
        act_down();

        actTimer = get_ticks()+(timeMod);

        while(get_ticks()<actTimer){
            //wait
        }
        //Toggle DOWN and UP for half a second (UP)
        act_down();
        act_up();

        actTimer = get_ticks()+(timeMod);

        while(get_ticks()<actTimer){
            //wait
        }
        count ++;
    }

    act_up();

    actuator = &act_reset;


}

void act_random_drop(){
    getRndNum(&randomNumber);
    act_up();

    actTimer = get_ticks()+interpolateNum(MIN_DROP_MS,MAX_DROP_MS,randomNumber);
    while (get_ticks()<actTimer)
    {
        /* code */
    }
    act_up();
    actuator = &act_reset;
    
}

//I currently don't want both to run at once
//toggles the actuator up
void act_up(){

    if(!is_down){
    //toggle Normally Open relay for UP actuator 
        PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14;
        PORT_REGS->GROUP[1].PORT_OUTTGL = PORT_PB06;
    is_up = !is_up;
    }
}

//toggles the actuator down
void act_down(){

    if(!is_up){
        PORT_REGS->GROUP[0].PORT_OUTSET = PORT_PA14;
        PORT_REGS->GROUP[1].PORT_OUTTGL = PORT_PB07;
        is_down = !is_down;
    }
}
void act_up_time(uint32_t time){
    

    volatile uint32_t elapse = get_ticks()+time;
 
    act_up();
    while(get_ticks()<elapse) {
        //wait
    }
    //toggle act up to off
    act_up();
    

}

void act_slow_up(){
    volatile uint32_t nextWait = 0;
    volatile uint32_t nextUp = 0;
    actTimer = get_ticks()+MS_SLOW_ELSAPSE;
    while (get_ticks()<actTimer){
        nextUp = get_ticks()+MS_SLOW_UP;
        act_up();
        while (get_ticks()<nextUp){
            /* code */
        }
        nextWait = get_ticks()+MS_SLOW_WAIT;
        while(get_ticks()<nextWait){


        }
        
    }
    
    
    actuator = &act_reset;
}

void act_quick_up(){
    actTimer = get_ticks()+MS_PER_SECOND;
    act_up();
    while(get_ticks()<actTimer){
        //wait
    }
    act_up();

    actuator = &act_reset;

}

void act_reset(){

    actTimer = get_ticks()+MS_PER_SECOND;

    //toggle Normally Open relay for DOWN actuator 
    act_down();
    while(get_ticks()<actTimer){
        //wait
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
    //PORT_REGS->GROUP[1].PORT_OUTTGL = PORT_PB06;
    PORT_REGS->GROUP[0].PORT_OUTSET = PORT_PA14;

\

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
    //PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14;
    // clear the interrupt! and go to the next operating mode
    EIC_REGS->EIC_INTFLAG |= EXTINT15_MASK;
}

uint32_t interpolateNum(uint32_t min, uint32_t max, uint32_t number){
    float out = (float)min;
    float adjust = (float)(max-min);
    adjust = adjust/(float)INT32_MAX;
    adjust = adjust*(float)number;
    out = out + adjust;
    return (uint32_t)out;


}

int main(void)
{
#ifndef NDEBUG
    for (int i = 0; i < DEBUG_WAIT; i++);
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
    dbg_write_str("~~~DEBUG ENABLED~~~\n");
#endif

    //PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14;
    //Relay ports

    // sleep until we have an interrupt



    
    
    uint32_t rndcount = 0;
    getRndNum(&randomNumber);
    //randomNumber = randomNumber&0x7;
    //randomNumber = randomNumber&0x3F;
    randomNumber = interpolateNum(2,6,randomNumber);
    while (1) {
        __WFI();
        actuator();

/*         if (((get_ticks() % (LED_FLASH_MS)) == 0) && rndcount < randomNumber) {
            PORT_REGS->GROUP[0].PORT_OUTTGL = PORT_PA14;
            rndcount ++;

        }  */
        
        if((get_ticks() % START_MS == 0)){
            actuator = actProgs[act_index];
            act_index = (act_index+1)%ACT_STATES;       
        }






/*         #ifndef NDEBUG
            if((get_ticks() % LED_FLASH_MS) == 0){
                dbg_write_str("Gyro x y z:");
                dbg_write_u16(gyro_xyz_buff,3);
                dbg_write_str(" \n");\
                
                dbg_write_str("XL x y z:");
                dbg_write_u16(xl_xyz_buff,3);
                dbg_write_str(" \n");

                uint16_t rpm = getRpm();
                dbg_write_str("Fan rpm:");
                dbg_write_u16(&rpm,1);
                dbg_write_str(" \n");
            }
        #endif   */

        
    }
    return 0;
}

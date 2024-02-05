#include "can.h"

#include <sam.h>

void canInit()
{
    // setup TX port
    PORT_REGS->GROUP[0].PORT_DIRSET      = PORT_PA22;
    PORT_REGS->GROUP[0].PORT_PINCFG[22] |= PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[0].PORT_PMUX[11]   |= PORT_PMUX_PMUXE_I;

    // setup RX port
    PORT_REGS->GROUP[0].PORT_DIRCLR      = PORT_PA23;
    PORT_REGS->GROUP[0].PORT_PINCFG[23] |= PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[0].PORT_PMUX[11]   |= PORT_PMUX_PMUXO_I;

    // setup clock
    GCLK_REGS->GCLK_PCHCTRL[27] = GCLK_PCHCTRL_CHEN_Msk;
    while ((GCLK_REGS->GCLK_PCHCTRL[27] & GCLK_PCHCTRL_CHEN_Msk) !=
           GCLK_PCHCTRL_CHEN_Msk)
        ;
    MCLK_REGS->MCLK_AHBMASK |= MCLK_AHBMASK_CAN0_Msk;

    // Configuration
    CAN0_REGS->CAN_CCCR |= CAN_CCCR_INIT_Msk;
    CAN0_REGS->CAN_CCCR |= CAN_CCCR_CCE_Msk | CAN_CCCR_TEST_Msk;

    CAN0_REGS->CAN_TEST = CAN_TEST_LBCK_Msk; // Enable loopback mode
    // TODO set filter config?
    // TODO I think Rx Buffer configuration is fine by default?
    CAN0_REGS->CAN_TXBC = CAN_TXBC_TFQS(32); // TODO I think we want this?

    // unset CCCR.INIT once synchronized
    while ((CAN0_REGS->CAN_CCCR & CAN_CCCR_INIT_Msk) != CAN_CCCR_INIT_Msk)
        ;
    CAN0_REGS->CAN_CCCR &= ~CAN_CCCR_INIT_Msk;
}

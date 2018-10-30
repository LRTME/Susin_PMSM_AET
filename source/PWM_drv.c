/************************************************************** 
* FILE:         PWM_drv.c
* DESCRIPTION:  PWM driver
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include "PWM_drv.h"

/**************************************************************
* PWM initialization
**************************************************************/
void PWM_init(void)
{
    // setup timer base 
    EPwm1Regs.TBPRD = PWM_PERIOD/2;       
    EPwm1Regs.TBCTL.bit.PHSDIR = TB_UP;
    EPwm1Regs.TBCTL.bit.CLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.HSPCLKDIV = TB_DIV1;
    EPwm1Regs.TBCTL.bit.SYNCOSEL = TB_CTR_ZERO;
    EPwm1Regs.TBCTL.bit.PRDLD = TB_SHADOW;
    EPwm1Regs.TBCTL.bit.PHSEN = TB_DISABLE;
    EPwm1Regs.TBCTR = 1;

    // debug mode behaviour
    #if PWM_DEBUG == 0
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 0;  // stop immediately
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 0;  // stop immediately
    #endif
    #if PWM_DEBUG == 1
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 1;  // stop when finished
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 1;  // stop when finished
    #endif
    #if PWM_DEBUG == 2
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 3;  // run free
    EPwm1Regs.TBCTL.bit.FREE_SOFT = 3;  // run free
    #endif

    // Compare registers
    EPwm1Regs.CMPA.bit.CMPA = PWM_PERIOD/4;

    // Init Action Qualifier Output A Register 
    EPwm1Regs.AQCTLA.bit.CAU = AQ_CLEAR;  // clear output on CMPA_UP
    EPwm1Regs.AQCTLA.bit.CAD = AQ_SET;    // set output on CMPA_DOWN

    // Dead Time
    
    // Trip zone 

    PWM_update(0.0);

    // output pin setup
    EALLOW;
    // GPIO0 pin is under ePWM control
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;
    EDIS;

}   //end of PWM_PWM_init

/**************************************************************
* PWM duty cycle update
* arg1: duty - Per Unit duty cycle [0.0, 1.0]
**************************************************************/
void PWM_update(float duty)
{
    int compare;

    // saturation
    if (duty < 0.0) duty = 0.0;
    if (duty > 1.0) duty = 1.0;

    // get the compare value
    compare = (int)((PWM_PERIOD/2) * duty);
    EPwm1Regs.CMPA.bit.CMPA = compare;
}  //end of PWM_update

/**************************************************************
* Set PWM period in seconds
* arg1: period - PWM period in seconds
**************************************************************/
void PWM_period(float period)
{
    // local variables
    float   temp_tbper;
    static float ostanek = 0;
    long celi_del;

    // calcualte TBPER (CPU_FREQ * period)
    temp_tbper = period * PWM_TMR_FREQ/2;
    
    // get the integer
    celi_del = (long)temp_tbper;
    // acumulate decimal part
    ostanek = ostanek + temp_tbper - celi_del;
    // act upon overflow of decimal part
    if (ostanek > 1.0)
    {
        ostanek = ostanek - 1.0;
        celi_del = celi_del + 1;
    }
    
    // set the period register
    EPwm1Regs.TBPRD = celi_del;
}   //end of PWM_period

/**************************************************************
* Set PWM period in Hz
* arg1: period - PWM period in Hz
**************************************************************/
void PWM_frequency(float frequency)
{
    PWM_period(1.0/frequency);
}   //end of PWM_frequency
  
/**************************************************************
* Start PWM timer
**************************************************************/
void PWM_start(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    EPwm1Regs.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}   //end of PWM_start




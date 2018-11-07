/****************************************************************
* FILENAME:     main.c
* DESCRIPTION:  initialization code
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include    "main.h"

// Global variables

// function declaration

/**************************************************************
* main function only executes initialization code
**************************************************************/
void main(void)
{
    // local variables


    // initialize system clock
    InitSysCtrl();
    EALLOW;
    ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 0;
    EDIS;

    // default GPIO initialization
    InitGpio();

    // initialize specific GPIO functions
    PCB_init();

    // initialize PIE expansion unit
    InitPieCtrl();

    // populate vector table with dummy interrupt functions
    InitPieVectTable();

    // communication stack initialization
    COMM_initialization();

    // initialize ADC and PWM
    ADC_init();
    SVM_init();

    // initialize periodic interrupt function
    PER_int_setup();

    // enable interrupts
    EINT;
    ERTM;

    // start timer, which will trigger ADC and an interrupt
    SVM_start();

    // proceed to background loop code
    BACK_loop();

}   // end of main


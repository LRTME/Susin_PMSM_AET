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

    // initialize overcurrent protection
    TRIP_OC_init();

    // initialize PIE expansion unit
    InitPieCtrl();

    // populate vector table with dummy interrupt functions
    InitPieVectTable();

    // communication stack initialization
    COMM_initialization();

    // initialize ADC and PWM
    ADC_init();
    SVM_init();

    // initialize QEP and SPI
    QEP_init(4*2500L); 		// incremental encoder has 10000 pulses per revolution
    SPI_init(8, 1, 1, 0); 	// SPI_init(int bits, long clock, int clk_phase, int clk_polarity)

    // enable overcurrent protection via trip zone
    TRIP_OC_enable();

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


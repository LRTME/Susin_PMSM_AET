/**************************************************************
* FILE:         CAP_drv.c
* DESCRIPTION:  Capture unit driver
* AUTHOR:       Mitja Nemec
* START DATE:   19.1.2012
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE        WHO                 DETAIL
* 1.0       19.1.2012  Mitja Nemec         Initial version
*
****************************************************************/
#include "CAP_drv.h"

int     CAP_prescalar;

/**************************************************************
* Inicializacija CAP modula
**************************************************************/
void CAP_init(void)
{
    // input pin cofiguration
    EALLOW;
    InputXbarRegs.INPUT7SELECT = 6; 			// GPIO 6 is used for CAPture module
    GpioCtrlRegs.GPAQSEL2.bit.GPIO24 = CAP_QUALIFICATION;   // 2: Qualification using 6 samples or 0: no qualification
    EDIS;

	CAP_MODUL.ECCTL1.bit.CAP1POL = 0;           // trigger capture on rising edge
	CAP_MODUL.ECCTL1.bit.CAP2POL = 0;
	CAP_MODUL.ECCTL1.bit.CAP3POL = 0;
	CAP_MODUL.ECCTL1.bit.CAP4POL = 0;
	CAP_MODUL.ECCTL1.bit.CTRRST1 = 1;           // reset counter after capture
	CAP_MODUL.ECCTL1.bit.CTRRST2 = 1;
	CAP_MODUL.ECCTL1.bit.CTRRST3 = 1;
	CAP_MODUL.ECCTL1.bit.CTRRST4 = 1;
	CAP_MODUL.ECCTL1.bit.PRESCALE = CAP_PRESCALER/2; // prescalar 2*n (n = 0:31)
	CAP_MODUL.ECCTL1.bit.CAPLDEN = 1;           // enable loading in four temporary registers

	#if CAP_DEBUG == 0
    CAP_MODUL.ECCTL1.bit.FREE_SOFT = 1;         // stop after current cycle
    #endif
    #if CAP_DEBUG == 1
    CAP_MODUL.ECCTL1.bit.FREE_SOFT = 0;         // stop after current cycle
    #endif
    #if CAP_DEBUG == 2
    CAP_MODUL.ECCTL1.bit.FREE_SOFT = 3;         // run free
    #endif
	
	CAP_MODUL.ECCTL2.bit.STOP_WRAP = 0;         // wrap after overflow on CAP1 (begin from 0 again)
	CAP_MODUL.ECCTL2.bit.CAP_APWM = 0;          // CAP mode selected
	CAP_MODUL.ECCTL2.bit.CONT_ONESHT = 0;       // contionous mode selected
	CAP_MODUL.ECCTL2.bit.SYNCO_SEL = 2;         // disable sync out signal
	CAP_MODUL.ECCTL2.bit.SYNCI_EN = 0;          // disable sync in option
	CAP_MODUL.ECCTL2.bit.TSCTRSTOP = 1;         // TSCTR (counter) free-running (freeze control)
}

/**************************************************************
* Funkcija ki vrne periodo signala na vhodu CAP enote
**************************************************************/
float CAP_period(void)
{
	unsigned long cycle_number;
    float   perioda;

    // izraèun prescalar-ja, ki pa ni enak CAP_PRESCALE (v registru)
    CAP_prescalar = CAP_MODUL.ECCTL1.bit.PRESCALE;

    if (CAP_prescalar == 0)
    {
        CAP_prescalar = 1;
    }
    else
    {
    	// korekcija prescalar-ja, tako da je vrednost enaka CAP_PRESCALE
        CAP_prescalar = 2*CAP_prescalar;
    }


    cycle_number = CAP_MODUL.CAP1;

    if (CAP_MODUL.ECFLG.bit.CTROVF == 0)
    {
        if (CAP_MODUL.TSCTR > (CPU_FREQ/CAP_FREQ_MIN))
        {
            perioda = 0.0;
        }
        else
        {
            perioda = cycle_number * (1.0/CPU_FREQ) * (1.0/CAP_prescalar);
        }
        
    }
    else
    {
        perioda = 0.0;
        CAP_MODUL.ECCLR.bit.CTROVF = 1;
    }
    
    return (perioda);
}


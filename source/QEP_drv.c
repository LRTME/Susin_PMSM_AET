/************************************************************** 
* FILE:         QEP_drv.c
* DESCRIPTION:  Inkrementalni dajalnik (QEP modul)
* AUTHOR:       Mitja Nemec
*
**************************************************************/
#include "QEP_drv.h"


/**************************************************************
* Funkcija, ki popise registre za QEP modul, omogoci vhode za
* QEP, omogoci position interrupt, omogoci position counter...
* return: void
**************************************************************/
void QEP_init(long pulses)
{
	//za QEP modul
    //omogocimo vhode v modul
    EALLOW;
    GpioCtrlRegs.GPACTRL.bit.QUALPRD1 = 0; // Qualification sampling period for GPIO0 to GPIO7 - lahko problem, èe ni implementiran diferencialno
    GpioCtrlRegs.GPACTRL.bit.QUALPRD2 = 0; // Qualification sampling period for GPIO0 to GPIO7 - lahko problem, èe ni implementiran diferencialno
    GpioCtrlRegs.GPAQSEL1.bit.GPIO6 = 0;    // sync GPIO6 to SYSCLK  (EQEP3A)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO7 = 0;    // sync GPIO7 to SYSCLK  (EQEP3B)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO8 = 0;    // sync GPIO8 to SYSCLK  (EQEP3S)
    GpioCtrlRegs.GPAQSEL1.bit.GPIO9 = 0;    // sync GPIO9 to SYSCLK  (EQEP3I)
    EDIS;

    GPIO_SetupPinMux(6, GPIO_MUX_CPU1, 5); 				// GPIO multiplexing (GPIO_NUM, CPU_SEL, GPIO_index) - GPIO6 is EQEP3A
    GPIO_SetupPinOptions(6, GPIO_INPUT, GPIO_INPUT);  	// GPIO direction    (GPIO_NUM, GPIO_DIR, GPIO_flag)

    GPIO_SetupPinMux(7, GPIO_MUX_CPU1, 5); 				// GPIO multiplexing (GPIO_NUM, CPU_SEL, GPIO_index) - GPIO7 is EQEP3B
    GPIO_SetupPinOptions(7, GPIO_INPUT, GPIO_INPUT);  	// GPIO direction    (GPIO_NUM, GPIO_DIR, GPIO_flag)

    GPIO_SetupPinMux(8, GPIO_MUX_CPU1, 5); 				// GPIO multiplexing (GPIO_NUM, CPU_SEL, GPIO_index) - GPIO8 is EQEP3S
    GPIO_SetupPinOptions(8, GPIO_INPUT, GPIO_INPUT);  	// GPIO direction    (GPIO_NUM, GPIO_DIR, GPIO_flag)

    GPIO_SetupPinMux(9, GPIO_MUX_CPU1, 5); 				// GPIO multiplexing (GPIO_NUM, CPU_SEL, GPIO_index) - GPIO9 is EQEPI
    GPIO_SetupPinOptions(9, GPIO_INPUT, GPIO_INPUT);  	// GPIO direction    (GPIO_NUM, GPIO_DIR, GPIO_flag)


	// nastavimo QEP2 modul
    QEP_MODUL1.QEPCTL.bit.FREE_SOFT = 0;
    QEP_MODUL1.QEPCTL.bit.PCRM = 1;	//counter resets on the maximum position
    QEP_MODUL1.QDECCTL.bit.SWAP = 0; // EQEPA and EQEPB signals are not swapped
    QEP_MODUL1.QDECCTL.bit.QAP = 0;  // EQEPA not negated
    QEP_MODUL1.QDECCTL.bit.QBP = 0;  // EQEPB not negated
    // nastavimo inicializacijsko vrednost POSITION COUNTERJA
    QEP_MODUL1.QPOSINIT = 0L;
    // nastavimo periodo position counterja
    QEP_MODUL1.QPOSMAX = pulses - 1;
    // enable position counter
    QEP_MODUL1.QEPCTL.bit.QPEN = 1; // QEP enable
    QEP_MODUL1.QPOSCTL.bit.PCSHDW = 0; // shadow disabled compare
    QEP_MODUL1.QPOSCTL.bit.PCE = 1; // enable Position-Compare
    QEP_MODUL1.QCAPCTL.bit.CEN = 1; // enable QEP capture

  	
}   // end of QEP_init

/**************************************************************
* Funkcija, ki vrne PPR (NUMBER OF PULSES PER POSITION)
* return: pulses per revolution
**************************************************************/
#pragma CODE_SECTION(QEP_ppr, "ramfuncs");
long QEP_ppr(void)
{
    long ppr;

    ppr = QEP_MODUL1.QPOSMAX + 1;
    ppr = ppr / 4;
    return(ppr);
}

/**************************************************************
* Funkcija, ki resetira position counter za QEP modul
* return: void
**************************************************************/
#pragma CODE_SECTION(QEP_reset, "ramfuncs");
void QEP_reset(void)
{
    QEP_MODUL1.QPOSCNT = 0;
    QEP_MODUL1.QPOSINIT = 0;
    QEP_MODUL1.QEPCTL.bit.SWI = 1;
    asm(" NOP");
    QEP_MODUL1.QEPCTL.bit.SWI = 0;
} // end of QEP_reset

/**************************************************************
* Funkcija, ki setira position counter za QEP modul
* return: void
**************************************************************/
#pragma CODE_SECTION(QEP_cntset, "ramfuncs");
void QEP_cntset(long count)
{
    QEP_MODUL1.QPOSCNT = count;
    QEP_MODUL1.QPOSINIT = count;
    QEP_MODUL1.QEPCTL.bit.SWI = 1;
    asm(" NOP");
    QEP_MODUL1.QEPCTL.bit.SWI = 0;
} // end of QEP2_set

/**************************************************************
* Funkcija, ki iz position counterja izracuna mehanski kot
* per unit(0.0 do 1.0)
* return: mehanski kot per unit od 0.0 do 1.0 (format IQ)
**************************************************************/
#pragma CODE_SECTION(QEP_mehKot, "ramfuncs");
float QEP_mehKot(void)
{
    float temp;
	

	//izracun mehanskega kota
    temp = (float)QEP_MODUL1.QPOSCNT / (float)(QEP_MODUL1.QPOSMAX + 1);

    return(temp);
}   // end of QEP_mehKot

/**************************************************************
* Funkcija, ki vrne vrednost position counterja (QPOSCNT)
* return: vrednost position counterja (QPOSCNT) (format int)
**************************************************************/
#pragma CODE_SECTION(QEP_cnt, "ramfuncs");
int QEP_cnt(void)
{
    return(QEP_MODUL1.QPOSCNT);
}   // end of QEP_cnt

/**************************************************************
* Funkcija, ki vrne smer vrtenja
* return: smer vrtenja (format int)
**************************************************************/
#pragma CODE_SECTION(QEP_dir, "ramfuncs");
int QEP_dir(void)
{
    //static int oldQEPdir;
    int QEPdir;

	// gledamo bit v registru
    QEPdir = QEP_MODUL1.QEPSTS.bit.QDF;
    
    if (QEPdir == 1)
    {
        return (+1);
    }
    else
    {
        return (-1);
    }
}   // end of QEP_dir





/****************************************************************
* FILENAME:     SVM_drv.c
* DESCRIPTION:  SVM HW driver
* AUTHOR:       Mitja Nemec
* START DATE:   16.1.2009
* VERSION:      2.1
* CHANGES:
*               Denis Susin     13.11.2018    Implementacija lastne funkcije SVM_update in
*               							  popravki kode za potrebe procesorja Delfino TMSF28379D in
*               							  preurejen vrstni red funkcij
****************************************************************/
#include "SVM_drv.h"

enum SVM_STATE svm_status = DISABLE;

/**************************************************************
* Inicializacija PWM modula za SVM
* returns: void
**************************************************************/
void SVM_init(void)
{
// period, compare & phase values
    // Init Timer-Base Period Register for EPWM1-EPWM3
    SVM_MODUL1.TBPRD = SVM_PERIOD/2;
    SVM_MODUL2.TBPRD = SVM_PERIOD/2;
    SVM_MODUL3.TBPRD = SVM_PERIOD/2;

    // setup timer base
    SVM_MODUL1.TBCTL.bit.PHSDIR = 1;     // count up after sync
    SVM_MODUL1.TBCTL.bit.CLKDIV = 0;
    SVM_MODUL1.TBCTL.bit.HSPCLKDIV = 0;
    SVM_MODUL1.TBCTL.bit.SYNCOSEL = 1;   // sync out on zero
    SVM_MODUL1.TBCTL.bit.PRDLD = 0;      // shadowed period reload
    SVM_MODUL1.TBCTL.bit.PHSEN = 0;      // master timer does not sync

    SVM_MODUL2.TBCTL.bit.PHSDIR = 1;     // count up after sync
    SVM_MODUL2.TBCTL.bit.CLKDIV = 0;
    SVM_MODUL2.TBCTL.bit.HSPCLKDIV = 0;
    SVM_MODUL2.TBCTL.bit.SYNCOSEL = 0;   // sync out is sync in
    SVM_MODUL2.TBCTL.bit.PRDLD = 0;      // shadowed period reload
    SVM_MODUL2.TBCTL.bit.PHSEN = 0;      // slave timer does sync

    SVM_MODUL3.TBCTL.bit.PHSDIR = 1;     // count up after sync
    SVM_MODUL3.TBCTL.bit.CLKDIV = 0;
    SVM_MODUL3.TBCTL.bit.HSPCLKDIV = 0;
    SVM_MODUL3.TBCTL.bit.SYNCOSEL = 3;   // sync out is disabled
    SVM_MODUL3.TBCTL.bit.PRDLD = 0;      // shadowed period reload
    SVM_MODUL3.TBCTL.bit.PHSEN = 0;      // slave timer does sync

    // debug mode behafiour
    #if SVM_DEBUG_MODE == 0
    SVM_MODUL1.TBCTL.bit.FREE_SOFT = 1;  // stop after current cycle
    SVM_MODUL2.TBCTL.bit.FREE_SOFT = 1;  // stop after current cycle
    SVM_MODUL3.TBCTL.bit.FREE_SOFT = 1;  // stop after current cycle
    #endif
    #if SVM_DEBUG_MODE == 1
    SVM_MODUL1.TBCTL.bit.FREE_SOFT = 0;  // stop after current cycle
    SVM_MODUL2.TBCTL.bit.FREE_SOFT = 0;  // stop after current cycle
    SVM_MODUL3.TBCTL.bit.FREE_SOFT = 0;  // stop after current cycle
    #endif
    #if SVM_DEBUG_MODE == 2
    SVM_MODUL1.TBCTL.bit.FREE_SOFT = 3;  // run free
    SVM_MODUL2.TBCTL.bit.FREE_SOFT = 3;  // run free
    SVM_MODUL3.TBCTL.bit.FREE_SOFT = 3;  // stop after current cycle
    #endif

    // Init Timer-Base Phase Register for EPWM1-EPWM3
    SVM_MODUL1.TBPHS.bit.TBPHS = 0;
    SVM_MODUL2.TBPHS.bit.TBPHS = 0;
    SVM_MODUL3.TBPHS.bit.TBPHS = 0;

    // compare setup
    SVM_MODUL1.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // re-load on zero
    SVM_MODUL1.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // shadowed compare reload

    SVM_MODUL2.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // re-load on zero
    SVM_MODUL2.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // shadowed compare reload

    SVM_MODUL3.CMPCTL.bit.LOADAMODE = CC_CTR_ZERO; // re-load on zero
    SVM_MODUL3.CMPCTL.bit.SHDWAMODE = CC_SHADOW; // shadowed compare reload

    SVM_MODUL1.CMPA.bit.CMPA = SVM_PERIOD/4; // 50% duty cycle
    SVM_MODUL2.CMPA.bit.CMPA = SVM_PERIOD/4; // 50% duty cycle
    SVM_MODUL3.CMPA.bit.CMPA = SVM_PERIOD/4; // 50% duty cycle

    // action qualifier setup
    SVM_MODUL1.AQSFRC.bit.RLDCSF = 0;
    SVM_MODUL2.AQSFRC.bit.RLDCSF = 0;
    SVM_MODUL3.AQSFRC.bit.RLDCSF = 0;

    SVM_MODUL1.AQCTLA.bit.CAU = AQ_SET;     // set output on CMPA_UP
    SVM_MODUL1.AQCTLA.bit.CAD = AQ_CLEAR;   // clear output on CMPA_DOWN
    SVM_MODUL1.AQCTLB.bit.CAU = AQ_SET;     // set output on CMPA_UP, ePWMB is temporary equal to ePWMA
    SVM_MODUL1.AQCTLB.bit.CAD = AQ_CLEAR;   // clear output on CMPA_DOWN, ePWMB is temporary equal to ePWMA

    SVM_MODUL2.AQCTLA.bit.CAU = AQ_SET;     // clear output on CMPA_UP
    SVM_MODUL2.AQCTLA.bit.CAD = AQ_CLEAR;   // set output on CMPA_DOWN
    SVM_MODUL2.AQCTLB.bit.CAU = AQ_SET;     // set output on CMPA_UP, ePWMB is temporary equal to ePWMA
    SVM_MODUL2.AQCTLB.bit.CAD = AQ_CLEAR;   // clear output on CMPA_DOWN, ePWMB is temporary equal to ePWMA

    SVM_MODUL3.AQCTLA.bit.CAU = AQ_SET;     // clear output on CMPA_UP
    SVM_MODUL3.AQCTLA.bit.CAD = AQ_CLEAR;   // set output on CMPA_DOWN
    SVM_MODUL3.AQCTLB.bit.CAU = AQ_SET;     // set output on CMPA_UP, ePWMB is temporary equal to ePWMA
    SVM_MODUL3.AQCTLB.bit.CAD = AQ_CLEAR;   // clear output on CMPA_DOWN, ePWMB is temporary equal to ePWMA

    // Init Dead-Band Generator Control Register for EPWM1-EPWM3
    SVM_MODUL1.DBCTL.bit.IN_MODE = 2;    // ePWMA and ePWMB are independable from each ather
    SVM_MODUL1.DBCTL.bit.POLSEL = 2;     // active high complementary mode, inverts ePWMB
    SVM_MODUL1.DBCTL.bit.OUT_MODE = 3;   // dead band on both outputs

    SVM_MODUL2.DBCTL.bit.IN_MODE = 2;    // ePWMA and ePWMB are independable from each ather
    SVM_MODUL2.DBCTL.bit.POLSEL = 2;     // active high complementary mode, inverts ePWMB
    SVM_MODUL2.DBCTL.bit.OUT_MODE = 3;   // dead band on both outputs

    SVM_MODUL3.DBCTL.bit.IN_MODE = 2;    // ePWMA and ePWMB are independable from each ather
    SVM_MODUL3.DBCTL.bit.POLSEL = 2;     // active high complementary mode, inverts ePWMB
    SVM_MODUL3.DBCTL.bit.OUT_MODE = 3;   // dead band on both outputs

    // Init Dead-Band Generator Falling/Rising Edge Delay Register for EPWM1-EPWM3
    SVM_dead_time(SVM_DEAD_TIME);

    // Do NOT use chopper module
    SVM_MODUL1.PCCTL.bit.CHPEN = 0;
    SVM_MODUL2.PCCTL.bit.CHPEN = 0;
    SVM_MODUL3.PCCTL.bit.CHPEN = 0;

    // trip zone functionality
    EALLOW;
    SVM_MODUL1.TZCTL.bit.TZA = 2;        // force low
    SVM_MODUL1.TZCTL.bit.TZB = 2;        // force low
    SVM_MODUL1.TZCLR.bit.OST = 1;        // clear any pending flags
    SVM_MODUL1.TZSEL.bit.OSHT1 = TZ_DISABLE;      // TZ1 triggers tripzone disabled

    SVM_MODUL2.TZCTL.bit.TZA = 2;        // force low
    SVM_MODUL2.TZCTL.bit.TZB = 2;        // force low
    SVM_MODUL2.TZCLR.bit.OST = 1;        // clear any pending flags
    SVM_MODUL2.TZSEL.bit.OSHT1 = TZ_DISABLE;      // TZ1 triggers tripzone disabled

    SVM_MODUL3.TZCTL.bit.TZA = 2;        // force low
    SVM_MODUL3.TZCTL.bit.TZB = 2;        // force low
    SVM_MODUL3.TZCLR.bit.OST = 1;        // clear any pending flags
    SVM_MODUL3.TZSEL.bit.OSHT1 = TZ_DISABLE;      // TZ1 triggers tripzone disabled
    EDIS;


    // event trigger module

    // output pin setup
    EALLOW;
    GpioCtrlRegs.GPAMUX1.bit.GPIO0 = 1;   // EPWM1A pin
    GpioCtrlRegs.GPAMUX1.bit.GPIO1 = 1;   // EPWM1B pin
    GpioCtrlRegs.GPAMUX1.bit.GPIO2 = 1;   // EPWM2A pin
    GpioCtrlRegs.GPAMUX1.bit.GPIO3 = 1;   // EPWM2B pin
    GpioCtrlRegs.GPAMUX1.bit.GPIO4 = 1;   // EPWM3A pin
    GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1;   // EPWM3B pin

    // input pin setup
    //GpioCtrlRegs.GPAPUD.bit.GPIO12 = 0; // enable pull up on TZ1
    //GpioCtrlRegs.GPAMUX1.bit.GPIO12 = 1;
    EDIS;                         // Disable EALLOW

    // privzeto je modul onemogoèen
    SVM_disable();
}

/**************************************************************
* Pozene casovnike za SVM modul
* returns: void
**************************************************************/
void SVM_start(void)
{
    EALLOW;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 0;
    SVM_MODUL1.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    SVM_MODUL2.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    SVM_MODUL3.TBCTL.bit.CTRMODE = TB_COUNT_UPDOWN;
    CpuSysRegs.PCLKCR0.bit.TBCLKSYNC = 1;
    EDIS;
}

/**************************************************************
* Vklopi vse izhode
* returns: void
**************************************************************/
#pragma CODE_SECTION(SVM_enable, "ramfuncs");
void SVM_enable(void)
{
    SVM_MODUL1.AQCSFRC.bit.CSFA = 0;	// forcing disabled
    SVM_MODUL1.AQCSFRC.bit.CSFB = 0;	// forcing disabled

    SVM_MODUL2.AQCSFRC.bit.CSFA = 0;	// forcing disabled
    SVM_MODUL2.AQCSFRC.bit.CSFB = 0;	// forcing disabled

    SVM_MODUL3.AQCSFRC.bit.CSFA = 0;	// forcing disabled
    SVM_MODUL3.AQCSFRC.bit.CSFB = 0;	// forcing disabled

    svm_status = ENABLE;
}

/**************************************************************
* Izklopi vse izhode
* returns: void
**************************************************************/
#pragma CODE_SECTION(SVM_disable, "ramfuncs");
void SVM_disable(void)
{
    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
    SVM_MODUL1.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
    SVM_MODUL2.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
    SVM_MODUL3.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

    svm_status = DISABLE;
}

/**************************************************************
* Izklopi vse tranzistorje - urgentno
* returns: void
**************************************************************/
void SVM_trip(void)
{
	EALLOW;
    SVM_MODUL1.TZFRC.bit.OST = 1;
    SVM_MODUL2.TZFRC.bit.OST = 1;
    SVM_MODUL3.TZFRC.bit.OST = 1;
    EDIS;

    svm_status = TRIP;
}

/**************************************************************
* Vklopi spodnje 3 tranzistorje
* returns: void
**************************************************************/
void SVM_bootstrap(void)
{
    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
    SVM_MODUL1.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output

    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
    SVM_MODUL2.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output

    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
    SVM_MODUL3.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output

    svm_status = BOOTSTRAP;
}

/**************************************************************
* Vklopi zgornje 3 tranzistorje
* returns: void
**************************************************************/
void SVM_high(void)
{
    SVM_MODUL1.AQCSFRC.bit.CSFA = 2;	// forcing high on PWMA output
    SVM_MODUL1.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

    SVM_MODUL2.AQCSFRC.bit.CSFA = 2;	// forcing high on PWMA output
    SVM_MODUL2.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

    SVM_MODUL3.AQCSFRC.bit.CSFA = 2;	// forcing high on PWMA output
    SVM_MODUL3.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

    svm_status = BOOTSTRAP;
}

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene periode
* in je natancna na cikel natancno
* returns: void
* arg1:    zelena perioda
**************************************************************/
#pragma CODE_SECTION(SVM_period, "ramfuncs");
void SVM_period(float perioda)
{
    // spremenljivke
    float   temp_tbper;
    static float ostanek = 0;
    long celi_del;

    // naracunam TBPER (CPU_FREQ * perioda)
    temp_tbper = perioda * CPU_FREQ/2;

    // izlocim celi del in ostanek
    celi_del = (long)temp_tbper;
    ostanek = temp_tbper - celi_del;
    // povecam celi del, ce je ostanek veji od 1
    if (ostanek > 1.0)
    {
        ostanek = ostanek - 1.0;
        celi_del = celi_del + 1;
    }

    // nastavim TBPER
    SVM_MODUL1.TBPRD = celi_del;
    SVM_MODUL2.TBPRD = celi_del;
    SVM_MODUL3.TBPRD = celi_del;
}   //end of FB_period

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene frekvence
* in je natancna na cikel natancno
* returns: void
* arg1:    zelena frekvenca
**************************************************************/
#pragma CODE_SECTION(SVM_frequency, "ramfuncs");
void SVM_frequency(float frekvenca)
{
    // spremenljivke
    float   temp_tbper;
    static float ostanek = 0;
    long celi_del;

    // naracunam TBPER (CPU_FREQ / SAMPLING_FREQ) - 1
    temp_tbper = (CPU_FREQ/2)/frekvenca;

    // izlocim celi del in ostanek
    celi_del = (long)temp_tbper;
    ostanek = temp_tbper - celi_del;
    // povecam celi del, ce je ostanek veji od 1
    if (ostanek > 1.0)
    {
        ostanek = ostanek - 1.0;
        celi_del = celi_del + 1;
    }

    // nastavim TBPER
    SVM_MODUL1.TBPRD = celi_del - 1;
    SVM_MODUL2.TBPRD = celi_del - 1;
    SVM_MODUL3.TBPRD = celi_del - 1;
}   //end of FB_frequency

/**************************************************************
* Funkcija, ki nastavi mrtvi cas v nanosekundah
* returns: void
**************************************************************/
#pragma CODE_SECTION(SVM_dead_time, "ramfuncs");
void SVM_dead_time(float dead_time)
{
    float cpu_cycle_time = (1.0/CPU_FREQ);
    
    long cycle_number;
    
    // naracunam koliko ciklov to znese
    cycle_number = (dead_time / 1000000000) / cpu_cycle_time;

    SVM_MODUL1.DBFED.all = cycle_number;
    SVM_MODUL1.DBRED.all = cycle_number;
    SVM_MODUL2.DBFED.all = cycle_number;
    SVM_MODUL2.DBRED.all = cycle_number;
    SVM_MODUL3.DBFED.all = cycle_number;
    SVM_MODUL3.DBRED.all = cycle_number;
}

/**************************************************************
* Funkcija, ki na podlagi vklopnega razmerja
* vklopi doloèene tranzistorje
* returns: void
* arg1:    vklopno razmerje [-1.0, +1.0] (IQ format)
**************************************************************/
#pragma CODE_SECTION(SVM_update_DC, "ramfuncs");
void SVM_update_DC(float duty)
{
    unsigned int compare1;
    unsigned int compare2;
    long delta;

    unsigned int perioda;



    // delam samo v primeru ce je mostic omogocen
    if (svm_status == ENABLE)
    {
        // zašèita za duty cycle
        //(zašèita za sektor je narejena v default switch case stavku)
        if (duty < -1.0) duty = -1.0;
        if (duty > +1.0) duty = +1.0;

        perioda = SVM_MODUL1.TBPRD;
        // koda da naracunam vrednost, ki bo sla v CMPR register
        delta = (perioda * duty)/2;

        compare1 = perioda/2 - delta;
        compare2 = perioda/2 + delta;

        // tretjo vejo za ziher izklopim
        SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
        SVM_MODUL3.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output 

        // vpisem v register
        SVM_MODUL1.CMPA.bit.CMPA = compare1;
        SVM_MODUL2.CMPA.bit.CMPA = compare2;
    }
}  //end of FB_update

/**************************************************************
* Na podlagi sektor in vklopnega razmerja nastavi PWM module
* returns: void
* arg1:    vklopno razmerje
* arg2:    sektor [1-6]
**************************************************************/
#pragma CODE_SECTION(SVM_update_six, "ramfuncs");
void SVM_update_six(float duty, int sektor)
{
    unsigned int compare;
    long delta;
    unsigned int perioda;
    int aq_bit = 0;

    // delam samo, ce je mostic omogocen
    if (svm_status == ENABLE)
    {
        // zasšita
        if (duty < 0.0) duty = 0.0;
        if (duty > 1.0) duty = 1.0;

        perioda = SVM_MODUL1.TBPRD;
        // koda da naracunam vrednost, ki bo sla v CMPR register
        delta = perioda * duty;

        compare = perioda - delta;

        // ko je compare tak da se eden od tranzistorjev ne vklopi, daj ven 100 duty cycle
        // if compare velu is close to zero, turn on during whole interval
        if (compare < (SVM_MODUL1.DBFED.all/2))
        {
            compare = 0;
            aq_bit = 2;						// it will force high on PWM output
        }
        // if compare value is close to PERIOD, switch off during whole interval
        if (compare > (perioda - SVM_MODUL1.DBFED.all/2))
        {
            compare = perioda + 1;
            aq_bit = 1;						// it will force low on PWM output
        }

        SVM_MODUL1.AQCSFRC.bit.CSFA = 0;	// forcing disabled
        SVM_MODUL1.AQCSFRC.bit.CSFB = 0;	// forcing disabled
        SVM_MODUL2.AQCSFRC.bit.CSFA = 0;	// forcing disabled
        SVM_MODUL2.AQCSFRC.bit.CSFB = 0;	// forcing disabled
        SVM_MODUL3.AQCSFRC.bit.CSFA = 0;	// forcing disabled
        SVM_MODUL3.AQCSFRC.bit.CSFB = 0;	// forcing disabled

        //
        switch(sektor)
        {
            case 1:                                             // voltage vector (1,0,0)
                    SVM_MODUL1.CMPA.bit.CMPA = compare;
                    SVM_MODUL1.AQCSFRC.bit.CSFA = aq_bit;
                    SVM_MODUL1.AQCSFRC.bit.CSFB = aq_bit;
                    SVM_MODUL2.CMPA.bit.CMPA = perioda;
                    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    SVM_MODUL3.CMPA.bit.CMPA = perioda;
                    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    break;
            case 2:                                             // voltage vector (1,1,0)
                    SVM_MODUL1.CMPA.bit.CMPA = compare;
                    SVM_MODUL1.AQCSFRC.bit.CSFA = aq_bit;
                    SVM_MODUL1.AQCSFRC.bit.CSFB = aq_bit;
                    SVM_MODUL2.CMPA.bit.CMPA = compare;
                    SVM_MODUL2.AQCSFRC.bit.CSFA = aq_bit;
                    SVM_MODUL2.AQCSFRC.bit.CSFB = aq_bit;
                    SVM_MODUL3.CMPA.bit.CMPA = perioda;
                    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    break;
            case 3:                                             // voltage vector (0,1,0)
                    SVM_MODUL1.CMPA.bit.CMPA = perioda;
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    SVM_MODUL2.CMPA.bit.CMPA = compare;
                    SVM_MODUL2.AQCSFRC.bit.CSFA = aq_bit;
                    SVM_MODUL2.AQCSFRC.bit.CSFB = aq_bit;
                    SVM_MODUL3.CMPA.bit.CMPA = perioda;
                    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    break;
            case 4:                                             // voltage vector (0,1,1)
                    SVM_MODUL1.CMPA.bit.CMPA = perioda;
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    SVM_MODUL2.CMPA.bit.CMPA = compare;
                    SVM_MODUL2.AQCSFRC.bit.CSFA = aq_bit;
                    SVM_MODUL2.AQCSFRC.bit.CSFB = aq_bit;
                    SVM_MODUL3.CMPA.bit.CMPA = compare;
                    SVM_MODUL3.AQCSFRC.bit.CSFA = aq_bit;
                    SVM_MODUL3.AQCSFRC.bit.CSFB = aq_bit;
                    break;
            case 5:                                             // voltage vector (0,0,1)
                    SVM_MODUL1.CMPA.bit.CMPA = perioda;
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    SVM_MODUL2.CMPA.bit.CMPA = perioda;
                    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    SVM_MODUL3.CMPA.bit.CMPA = compare;
                    SVM_MODUL3.AQCSFRC.bit.CSFA = aq_bit;
                    SVM_MODUL3.AQCSFRC.bit.CSFB = aq_bit;
                    break;
            case 6:                                             // voltage vector (1,0,1)
                    SVM_MODUL1.CMPA.bit.CMPA = compare;
                    SVM_MODUL1.AQCSFRC.bit.CSFA = aq_bit;
                    SVM_MODUL1.AQCSFRC.bit.CSFB = aq_bit;
                    SVM_MODUL2.CMPA.bit.CMPA = perioda;
                    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    SVM_MODUL3.CMPA.bit.CMPA = compare;
                    SVM_MODUL3.AQCSFRC.bit.CSFA = aq_bit;
                    SVM_MODUL3.AQCSFRC.bit.CSFB = aq_bit;
                    break;
            default:                                             // voltage vector (0,0,0)
                    SVM_MODUL1.CMPA.bit.CMPA = perioda;
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    SVM_MODUL2.CMPA.bit.CMPA = perioda;
                    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    SVM_MODUL3.CMPA.bit.CMPA = perioda;
                    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    break;
        }
    }
}


/**************************************************************
* Na podlagi sektor in vklopnega razmerja nastavi PWM module
* returns: void
* arg1:    vklopno razmerje
* arg2:    sektor [1-6]
**************************************************************/
#pragma CODE_SECTION(SVM_update_bldc, "ramfuncs");
void SVM_update_bldc(float svm_duty, int svm_sektor)
{       
    unsigned int compare;
    long delta;
    unsigned int perioda;

    // delam samo, ce je mostic omogocen
    if (svm_status == ENABLE)
    {
        svm_duty = - svm_duty;
        // preverim predznak
        if (svm_duty < 0)
        {
            svm_sektor = svm_sektor + 3;
            if (svm_sektor > 6)
            {
                svm_sektor = svm_sektor - 6;
            }
        }
    
        // zascita
        svm_duty = fabs(svm_duty);
        if (svm_duty > 1.0) svm_duty = 1.0;

        perioda = SVM_MODUL1.TBPRD;
        // koda da naracunam vrednost, ki bo sla v CMPR register
        delta = perioda * svm_duty;

        compare = perioda - delta;

        SVM_MODUL1.CMPA.bit.CMPA = compare;
        SVM_MODUL2.CMPA.bit.CMPA = compare;
        SVM_MODUL3.CMPA.bit.CMPA = compare;

        //
        switch(svm_sektor)
        {
            case 1:
                    // Z
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

                    // L
                    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output

                    // H
                    SVM_MODUL3.CMPA.bit.CMPA = compare;
                    SVM_MODUL3.AQCSFRC.bit.CSFA = 0;	// forcing disabled
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 0;	// forcing disabled
                    break;
            case 2:
                    // H
                    SVM_MODUL1.CMPA.bit.CMPA = compare;
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 0;	// forcing disabled
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 0;	// forcing disabled

                    // L
                    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output

                    // Z
                    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output
                    break;
            case 3:
                    // H
                    SVM_MODUL1.CMPA.bit.CMPA = compare;
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 0;	// forcing disabled
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 0;	// forcing disabled

                    // Z
                    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

                    // L
                    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    break;
            case 4:
                    // Z
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

                    // H
                    SVM_MODUL2.CMPA.bit.CMPA = compare;
                    SVM_MODUL2.AQCSFRC.bit.CSFA = 0;	// forcing disabled
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 0;	// forcing disabled

                    // L
                    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
                    break;
            case 5:
                    // L
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output

                    // H
                    SVM_MODUL2.CMPA.bit.CMPA = compare;
                    SVM_MODUL2.AQCSFRC.bit.CSFA = 0;	// forcing disabled
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 0;	// forcing disabled

                    // Z
                    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output
                    break;
            case 6:
                    // L
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output

                    // Z
                    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

                    // H
                    SVM_MODUL3.CMPA.bit.CMPA = compare;
                    SVM_MODUL3.AQCSFRC.bit.CSFA = 0;	// forcing disabled
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 0;	// forcing disabled
                    break;
            default:
                    SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL1.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

                    SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL2.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output

                    SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
                    SVM_MODUL3.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output
                    break;
        }
    }
}

/**************************************************************
* Na podlagi Ualpha in Ubeta nastavi PWM module
* returns: void
* arg1:    napetost Ualpha [-1.0, +1.0] (IQ format)
* arg2:    napetost Ubeta [-1.0, +1.0] (IQ format)
**************************************************************/
#pragma CODE_SECTION(SVM_update, "ramfuncs");
void SVM_update(float Ualpha, float Ubeta)
{

    /* lokalne spremenljivke */
    float d_sector;
    float d_sector_plus_1;
    float duty_a;
    float duty_b;
    float duty_c;
    int sector = 0;


    unsigned int compare;
    unsigned int perioda;
    long delta;

    // izvedem samo, ce je mostic omogocen
    if (svm_status == ENABLE)
    {

        // limitations ensurement
        if (Ualpha > (+1.0))
        {
            Ualpha = (+1.0);
        }
        if (Ualpha < (-1.0))
        {
            Ualpha = (-1.0);
        }
        if (Ubeta > (+1.0))
        {
            Ubeta = (+1.0);
        }
        if (Ubeta < (-1.0))
        {
            Ubeta = (-1.0);
        }
        

        // 60 degree sector determination
        if (Ubeta >= 0.0)                            // sectors 1, 2, 3
            {
            if (Ubeta < SQRT3*Ualpha)
            {
               sector = 1;
            }
            else if (Ubeta <= -SQRT3*Ualpha)
            {
               sector = 3;
            }
            else
            {
               sector = 2;
            }
        }
        else                                        // sectors 4, 5, 6
        {
            if (-Ubeta <= SQRT3*Ualpha)
            {
                sector = 6;
            }
            else if (-Ubeta < -SQRT3*Ualpha)
            {
                sector = 4;
            }
            else
            {
                sector = 5;
            }
        }


        if (Ubeta == 0.0 && Ualpha < 0.0)               // special condition for voltage vector (0,1,1), which is in sector 4
        {
            sector = 4;
        }

        if (Ubeta == 0.0 && Ualpha == 0.0)              // no voltage desired
        {
            sector = 0;
        }


        // d_sector, d_sector_plus_1 (duty_a,duty_b,duty_c) calculations
        if (sector == 0)  // sector 0: this is special case for (Ualpha,Ubeta) = (0,0); voltage vector is (0,0,0) and (1,1,1) - half of period each
        {
           duty_a = 0.5;
           duty_b = 0.5;
           duty_c = 0.5;
        }
        if (sector == 1)
        {
           d_sector = Ualpha - Ubeta/SQRT3;                             // d1
           d_sector_plus_1 = 2*Ubeta/SQRT3;                             // d2

           duty_c = (1.0 - d_sector - d_sector_plus_1) / 2;             // dc = d7
           duty_b = duty_c + d_sector_plus_1;                           // db = d2 + d7
           duty_a = duty_b + d_sector;                                  // da = d1 + d2 + d7
        }
        else if (sector == 2)
        {
            d_sector = Ualpha + Ubeta/SQRT3;                            // d2
            d_sector_plus_1 = -Ualpha + Ubeta/SQRT3;                    // d3

            duty_c = (1.0 - d_sector - d_sector_plus_1) / 2;            // dc = d7
            duty_a = duty_c + d_sector;                                 // da = d2 + d7
            duty_b = duty_a + d_sector_plus_1;                          // db = d2 + d3 + d7
        }      
        else if (sector == 3)
        {
            d_sector = 2*Ubeta/SQRT3;                                   // d3
            d_sector_plus_1 = -Ualpha - Ubeta/SQRT3;                    // d4

            duty_a = (1.0 - d_sector - d_sector_plus_1) / 2;            // da = d7
            duty_c = duty_a + d_sector_plus_1;                          // dc = d4 + d7
            duty_b = duty_c + d_sector;                                 // db = d3 + d4 + d7
        }   
        else if (sector == 4)
        {
            d_sector = -Ualpha + Ubeta/SQRT3;                           // d4
            d_sector_plus_1 = -2*Ubeta/SQRT3;                           // d5

            duty_a = (1.0 - d_sector - d_sector_plus_1) / 2;            // da = d7
            duty_b = duty_a + d_sector;                                 // db = d4 + d7
            duty_c = duty_b + d_sector_plus_1;                          // dc = d4 + d5 + d7
        }   
        else if (sector == 5)
        {
            d_sector = -Ualpha - Ubeta/SQRT3;                           // d5
            d_sector_plus_1 = Ualpha - Ubeta/SQRT3;                     // d6

            duty_b = (1.0 - d_sector - d_sector_plus_1) / 2;            // db = d7
            duty_a = duty_b + d_sector_plus_1;                          // da = d6 + d7
            duty_c = duty_a + d_sector;                                 // dc = d5 + d6 + d7
        }   
        else if (sector == 6)
        {
            d_sector = -2*Ubeta/SQRT3;                                  // d6
            d_sector_plus_1 = Ualpha + Ubeta/SQRT3;                     // d1

            duty_b = (1.0 - d_sector - d_sector_plus_1) / 2;            // db = d7
            duty_c = duty_b + d_sector;                                 // dc = d6 + d7
            duty_a = duty_c + d_sector_plus_1;                          // da = d1 + d6 + d7
        }


        // saturacija
        if (duty_a > 1.0) duty_a = 1.0;
        if (duty_b > 1.0) duty_b = 1.0;
        if (duty_c > 1.0) duty_c = 1.0;
        if (duty_a < 0.0) duty_a = 0.0;
        if (duty_b < 0.0) duty_b = 0.0;
        if (duty_c < 0.0) duty_c = 0.0;



        // koda da naracunam vrednost, ki bo sla v CMPR register
        /* first leg */
        perioda = SVM_MODUL1.TBPRD;

        delta = perioda * duty_a;

        compare = perioda - delta;

        SVM_MODUL1.AQCSFRC.bit.CSFA = 0;	// forcing disabled
        SVM_MODUL1.AQCSFRC.bit.CSFB = 0;	// forcing disabled
/*
        // if compare value is close to ZERO, turn on during whole interval
        if (compare < (SVM_MODUL1.DBFED.all/2))
        {
            compare = 0;
            SVM_MODUL1.AQCSFRC.bit.CSFA = 2;	// forcing high on PWMA output
            SVM_MODUL1.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output
        } 
        // if compare value is close to PERIOD, switch off during whole interval
        if (compare > (perioda - SVM_MODUL1.DBFED.all/2))
        {
            compare = perioda + 1;
            SVM_MODUL1.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
            SVM_MODUL1.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
        }
*/
        SVM_MODUL1.CMPA.bit.CMPA = compare;

        /* second leg */
        perioda = SVM_MODUL2.TBPRD;

        delta = perioda * duty_b;

        compare = perioda - delta;

        SVM_MODUL2.AQCSFRC.bit.CSFA = 0;	// forcing disabled
        SVM_MODUL2.AQCSFRC.bit.CSFB = 0;	// forcing disabled
/*
        // if compare value is close to ZERO, turn on during whole interval
        if (compare < (SVM_MODUL1.DBFED.all/2))
        {
            compare = 0;
            SVM_MODUL2.AQCSFRC.bit.CSFA = 2;	// forcing high on PWMA output
            SVM_MODUL2.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output
        } 
        // if compare value is close to PERIOD, switch off during whole interval
        if (compare > (perioda - SVM_MODUL1.DBFED.all/2))
        {
            compare = perioda + 1;
            SVM_MODUL2.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
            SVM_MODUL2.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
        }
*/
        SVM_MODUL2.CMPA.bit.CMPA = compare;

        /* third leg */
        perioda = SVM_MODUL3.TBPRD;

        delta = perioda * duty_c;

        compare = perioda - delta;

        SVM_MODUL3.AQCSFRC.bit.CSFA = 0;	// forcing disabled
        SVM_MODUL3.AQCSFRC.bit.CSFB = 0;	// forcing disabled
/*
        // if compare value is close to ZERO, turn on during whole interval
        if (compare < (SVM_MODUL1.DBFED.all/2))
        {
            compare = 0;
            SVM_MODUL3.AQCSFRC.bit.CSFA = 2;	// forcing high on PWMA output
            SVM_MODUL3.AQCSFRC.bit.CSFB = 2;	// forcing high on PWMB output
        } 
        // if compare value is close to PERIOD, switch off during whole interval
        if (compare > (perioda - SVM_MODUL1.DBFED.all/2))
        {
            compare = perioda + 1;
            SVM_MODUL3.AQCSFRC.bit.CSFA = 1;	// forcing low on PWMA output 
            SVM_MODUL3.AQCSFRC.bit.CSFB = 1;	// forcing high on PWMB output
        }
*/
        SVM_MODUL3.CMPA.bit.CMPA = compare;
    }    
}

/**************************************************************
* vrne status (delam/ne delam)
* returns: svm_status
**************************************************************/
int SVM_status(void)
{
    return(svm_status);
}

/**************************************************************
* Vrne vklopno razmerje veje
* returns: duty_leg
**************************************************************/
#pragma CODE_SECTION(SVM_get_duty_leg1, "ramfuncs");
float SVM_get_duty_leg1(void)
{
    return ((float)SVM_MODUL1.CMPA.bit.CMPA / (float)SVM_MODUL1.TBPRD);
}
#pragma CODE_SECTION(SVM_get_duty_leg2, "ramfuncs");
float SVM_get_duty_leg2(void)
{
    return ((float)SVM_MODUL2.CMPA.bit.CMPA / (float)SVM_MODUL1.TBPRD);
}
#pragma CODE_SECTION(SVM_get_duty_leg3, "ramfuncs");
float SVM_get_duty_leg3(void)
{
    return ((float)SVM_MODUL3.CMPA.bit.CMPA / (float)SVM_MODUL1.TBPRD);
}

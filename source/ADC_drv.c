/**************************************************************
* FILE:         ADC_drv.c
* DESCRIPTION:  A/D driver
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include "ADC_drv.h"

/**************************************************************
* initialization of ADC
**************************************************************/
void ADC_init(void)
{
    Uint16  acqps_min;
    float   acqps_set;
    float   t_sh;

    EALLOW;

    //write configurations
    AdcaRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcbRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdccRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcdRegs.ADCCTL2.bit.PRESCALE = 6; //set ADCCLK divider to /4
    AdcSetMode(ADC_ADCA, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCB, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCC, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);
    AdcSetMode(ADC_ADCD, ADC_RESOLUTION_12BIT, ADC_SIGNALMODE_SINGLE);

    //Set pulse positions to late
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdccRegs.ADCCTL1.bit.INTPULSEPOS = 1;
    AdcdRegs.ADCCTL1.bit.INTPULSEPOS = 1;

    //power up the ADCs
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdccRegs.ADCCTL1.bit.ADCPWDNZ = 1;
    AdcdRegs.ADCCTL1.bit.ADCPWDNZ = 1;

    // enable ADCtemp measurement
    AnalogSubsysRegs.TSNSCTL.bit.ENABLE = 1;

    //delay for 1ms to allow ADC time to power up
    DELAY_US(1000);

    EDIS;
    
    //determine minimum acquisition window (in SYSCLKS) based on resolution
    if(ADC_RESOLUTION_12BIT == AdcaRegs.ADCCTL2.bit.RESOLUTION)
    {
        acqps_min = 14; //75ns
    }
    //resolution is 16-bit
    else 
    { 
        acqps_min = 63; //320ns
    }

    // get the acqps for current hardware
    // revision B of the CPU -> time constant = 30 ns.
    // for 12-bit resolution aquisition should be no longer than 250 ns
    t_sh = 250; //ns

    acqps_set = t_sh*CPU_FREQ/1000000000 - 1;
    // round the number
    acqps_set = acqps_set + 0.5;

    // check for the minimum required time
    if (acqps_set < acqps_min)
    {
        acqps_set = acqps_min;
    }

    // ADCA channel setup
    EALLOW;
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 2;          //SOC0 will convert pin A2
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = acqps_set;  //sample window
    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 3;          //SOC1 will convert pin A3
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = acqps_set;  //sample window
    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 4;          //SOC2 will convert pin A4
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = acqps_set;  //sample window
    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcaRegs.ADCSOC3CTL.bit.CHSEL = 5;          //SOC3 will convert pin A5
    AdcaRegs.ADCSOC3CTL.bit.ACQPS = acqps_set;  //sample window
    AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcaRegs.ADCSOC4CTL.bit.CHSEL = 13;         //SOC4 will convert pin temperature
    AdcaRegs.ADCSOC4CTL.bit.ACQPS = 140;        //sample window 700 ns
    AdcaRegs.ADCSOC4CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    // SOC 3 will raise interrupt flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1SEL = 3;      //end of SOC3 will set INT1 flag
    AdcaRegs.ADCINTSEL1N2.bit.INT1E = 1;        //enable INT1 flag
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;      //make sure INT1 flag is cleared

    // ADCB channel setup
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = 2;          //SOC0 will convert pin B2
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = acqps_set;  //sample window
    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcbRegs.ADCSOC1CTL.bit.CHSEL = 3;          //SOC1 will convert pin B3
    AdcbRegs.ADCSOC1CTL.bit.ACQPS = acqps_set;  //sample window
    AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcbRegs.ADCSOC2CTL.bit.CHSEL = 14;          //SOC2 will convert pin 14
    AdcbRegs.ADCSOC2CTL.bit.ACQPS = acqps_set;  //sample window
    AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcbRegs.ADCSOC3CTL.bit.CHSEL = 15;          //SOC3 will convert pin 15
    AdcbRegs.ADCSOC3CTL.bit.ACQPS = acqps_set;  //sample window
    AdcbRegs.ADCSOC3CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    // SOC 4 will raise interrupt flag
    AdcbRegs.ADCINTSEL1N2.bit.INT2SEL = 3;      //end of SOC4 will set INT1 flag
    AdcbRegs.ADCINTSEL1N2.bit.INT2E = 1;        //enable INT2 flag
    AdcbRegs.ADCINTFLGCLR.bit.ADCINT2 = 1;      //make sure INT2 flag is cleared

    // ADCC channel setup
    AdccRegs.ADCSOC0CTL.bit.CHSEL = 2;          //SOC0 will convert pin C2
    AdccRegs.ADCSOC0CTL.bit.ACQPS = acqps_set;  //sample window
    AdccRegs.ADCSOC0CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdccRegs.ADCSOC4CTL.bit.CHSEL = 12;         //SOC4 will convert pin A12 - DACA
    AdccRegs.ADCSOC4CTL.bit.ACQPS = 511;        //sample window
    AdccRegs.ADCSOC4CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    // SOC 4 will raise interrupt flag
    AdccRegs.ADCINTSEL1N2.bit.INT2SEL = 4;      //end of SOC4 will set INT1 flag
    AdccRegs.ADCINTSEL1N2.bit.INT2E = 1;        //enable INT2 flag
    AdccRegs.ADCINTFLGCLR.bit.ADCINT2 = 1;      //make sure INT2 flag is cleared

    // ADCD channel setup
    AdcdRegs.ADCSOC0CTL.bit.CHSEL = 2;          //SOC0 will convert pin D2
    AdcdRegs.ADCSOC0CTL.bit.ACQPS = acqps_set;  //sample window
    AdcdRegs.ADCSOC0CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcdRegs.ADCSOC1CTL.bit.CHSEL = 3;          //SOC1 will convert pin D3
    AdcdRegs.ADCSOC1CTL.bit.ACQPS = acqps_set;  //sample window
    AdcdRegs.ADCSOC1CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcdRegs.ADCSOC2CTL.bit.CHSEL = 4;          //SOC2 will convert pin D4
    AdcdRegs.ADCSOC2CTL.bit.ACQPS = acqps_set;  //sample window
    AdcdRegs.ADCSOC2CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcdRegs.ADCSOC3CTL.bit.CHSEL = 5;          //SOC3 will convert pin D5
    AdcdRegs.ADCSOC3CTL.bit.ACQPS = acqps_set;  //sample window
    AdcdRegs.ADCSOC3CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    AdcdRegs.ADCSOC4CTL.bit.CHSEL = 12;         //SOC4 will convert pin A12 - DACA
    AdcdRegs.ADCSOC4CTL.bit.ACQPS = 511;        //sample window
    AdcdRegs.ADCSOC4CTL.bit.TRIGSEL = 5;        //trigger on ePWM1 SOCA/C

    // SOC 4 will raise interrupt flag
    AdcdRegs.ADCINTSEL1N2.bit.INT2SEL = 4;      //end of SOC4 will set INT1 flag
    AdcdRegs.ADCINTSEL1N2.bit.INT2E = 1;        //enable INT2 flag
    AdcdRegs.ADCINTFLGCLR.bit.ADCINT2 = 1;      //make sure INT2 flag is cleared

    EDIS;

    // ADC trigger setup
    // trigger when PWM timer is at zero
    ADC_MODUL1.ETSEL.bit.SOCASEL = ET_CTR_ZERO;
    // trigger every time
    ADC_MODUL1.ETPS.bit.SOCAPRD = 1;
    // clear any previous triggers
    ADC_MODUL1.ETCLR.bit.SOCA = 1;
    // enable trigger
    ADC_MODUL1.ETSEL.bit.SOCAEN = 1;

    // wait for stabilization
    DELAY_US(1000L);

    // Initialize temperature sensor
    InitTempSensor(3.0);

}   //end of AP_ADC_init

/**************************************************************
* Function which waits for the ADC to finish with current seqence
* return: void
**************************************************************/
#pragma CODE_SECTION(ADC_wait, "ramfuncs");
void ADC_wait(void)
{
    while (AdcaRegs.ADCINTFLG.bit.ADCINT1 == 0)
    {
        /* DO NoTHING */
    }
    AdcaRegs.ADCINTFLGCLR.bit.ADCINT1;
}

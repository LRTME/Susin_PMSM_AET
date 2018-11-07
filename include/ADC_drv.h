/**************************************************************
* FILE:         ADC_drv.h
* DESCRIPTION:  A/D driver
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef     __ADC_DRV_H__
#define     __ADC_DRV_H__

#include    "F28x_Project.h"
#include    "define.h"
#include    "DAC_drv.h"

// which PWM module acts as a trigger
#define     ADC_MODUL1      EPwm1Regs

// map the results registers to defines, for easier reconfiguration
#define     ADC_A2    		(AdcaResultRegs.ADCRESULT0)
#define     ADC_A3    		(AdcaResultRegs.ADCRESULT1)
#define     ADC_A4       	(AdcaResultRegs.ADCRESULT2)
#define     ADC_A5			(AdcaResultRegs.ADCRESULT3)
#define     ADC_TEMP        (AdcaResultRegs.ADCRESULT4)

#define     ADC_B2    		(AdcbResultRegs.ADCRESULT0)
#define     ADC_B3    		(AdcbResultRegs.ADCRESULT1)
#define     ADC_B4    		(AdcbResultRegs.ADCRESULT2)
#define     ADC_B5    		(AdcbResultRegs.ADCRESULT3)
#define     ADC_14       	(AdccResultRegs.ADCRESULT4)

#define     ADC_C2       	(AdccResultRegs.ADCRESULT0)
#define     ADC_C3       	(AdccResultRegs.ADCRESULT1)
#define     ADC_C4       	(AdccResultRegs.ADCRESULT2)
#define     ADC_C5       	(AdccResultRegs.ADCRESULT3)
#define     ADC_15       	(AdccResultRegs.ADCRESULT4)

#define     ADC_D2       	(AdcdResultRegs.ADCRESULT0)
#define     ADC_D3       	(AdcdResultRegs.ADCRESULT1)
#define     ADC_D4       	(AdcdResultRegs.ADCRESULT2)
#define     ADC_D5       	(AdcdResultRegs.ADCRESULT3)


/**************************************************************
* initialization of ADC
**************************************************************/
extern void ADC_init(void);

/**************************************************************
* Function which waits for the ADC A to finish with current seqence
* return: void
**************************************************************/
extern void ADC_A_wait(void);

/**************************************************************
* Function which waits for the ADC B to finish with current seqence
* return: void
**************************************************************/
extern void ADC_B_wait(void);

/**************************************************************
* Function which waits for the ADC C to finish with current seqence
* return: void
**************************************************************/
extern void ADC_C_wait(void);

/**************************************************************
* Function which waits for the ADC D to finish with current seqence
* return: void
**************************************************************/
extern void ADC_D_wait(void);

#endif /* __ADC_DRV_H__ */

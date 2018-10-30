/************************************************************** 
* FILE:         PWM_drv.h
* DESCRIPTION:  PWM driver
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef     __PWM_DRV_H__
#define     __PWM_DRV_H__

#include    "F28x_Project.h"

#include    "define.h"
#include    "globals.h"

/* Definicije */
// how PWM module behaves on debug event
// 0 stop immediately, 1 stop when finished, 2 run free
#define     PWM_DEBUG       0

// PWM unit internal frequency
#define		PWM_TMR_FREQ	(CPU_FREQ/2)

// PWM timer period
#define     PWM_PERIOD      (PWM_TMR_FREQ/(SAMPLE_FREQ * SAMP_PRESCALE))

/**************************************************************
* PWM initialization
**************************************************************/
extern void PWM_init(void);

/**************************************************************
* PWM duty cycle update
* arg1: duty - Per Unit duty cycle [0.0, 1.0]
**************************************************************/
extern void PWM_update(float duty);

/**************************************************************
* Start PWM timer
**************************************************************/
extern void PWM_start(void);

/**************************************************************
* Set PWM period in seconds
* arg1: period - PWM period in seconds
**************************************************************/
extern void PWM_period(float perioda);

/**************************************************************
* Set PWM period in Hz
* arg1: period - PWM period in Hz
**************************************************************/
extern void PWM_frequency(float frekvenca);

#endif  // end of __PWM_DRV_H__ definition


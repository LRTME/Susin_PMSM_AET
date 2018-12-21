/**************************************************************
* FILE:         CAP_drv.c
* DESCRIPTION:  Capture unit driver
* AUTHOR:       Mitja Nemec
* START DATE:   19.1.2012
* VERSION:      1.0
*
* CHANGES :
* VERSION   DATE        WHO                 DETAIL
* 1.0       19.1.2012   Mitja Nemec         Initial version
*
****************************************************************/
#ifndef __CAP_DRV_H__
#define __CAP_DRV_H__

#include    "F28x_Project.h"
#include    "define.h"
#include    "QEP_drv.h"

// katero enoto uporabljamo
#define     CAP_MODUL		ECap1Regs

// katera je najnizja frekvenca preden vrne napako
#define     CAP_FREQ_MIN    1

// delovanje modula ob debug-dogodkih
// (0-stop at zero, 1-stop immediately, 2-run free)
#define     CAP_DEBUG		 2

// Prescalar CAP enote - mora biti deljiv z 2 ali pa je 1
// (prescalar_min:1, prescalar_max:62)
#define     CAP_PRESCALER    2

// Qualification CAP enote (pove najmanj koliko èasa mora miniti med dvema frontama)
// (Qualification = {0-no qualification, 1-qualification 3 samples, 2-qualification 6 samples })
#define     CAP_QUALIFICATION    0


/**************************************************************
* Funkcija ki vrne periodo signala na vhodu CAP enote
**************************************************************/
extern float CAP_period(void);

/**************************************************************
* Inicializacija CAP modula
**************************************************************/
extern void CAP_init(void);


#endif /* __CAP_DRV_H__ */

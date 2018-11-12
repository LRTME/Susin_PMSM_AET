/************************************************************** 
* FILE:         QEP_drv.h
* DESCRIPTION:  Inkrementalni dajalnik (QEP modul)
* AUTHOR:       Mitja Nemec
*
**************************************************************/
#ifndef     __QEP_DRV_H__
#define     __QEP_DRV_H__

#include    "F28x_Project.h"
#include    "define.h"

#define		QEP_MODUL1	EQep3Regs

/**************************************************************
* Funkcije, ki inicializirajo QEP modul
**************************************************************/

/**************************************************************
* Funkcija, ki popiše registre za QEP modul, omogoèi vhode za
* QEP, omogoèi position interrupt, omogoèi position counter...
* return: void
**************************************************************/
extern void QEP_init(long pulses);

/**************************************************************
* Funkcija, ki resetira position counter za QEP modul
* return: void
**************************************************************/
extern void QEP_reset(void);

/**************************************************************
* Funkcija, ki postavi position counter za QEP modul
* vzame argument v P.U
* return: void
**************************************************************/
extern void QEP_mehset(float angle);

/**************************************************************
* Funkcija, ki postavi position counter za QEP modul
* return: void
**************************************************************/
extern void QEP_cntset(long count);

/**************************************************************
* Funkcija, ki iz position counterja izraèuna mehanski kot
* per unit(0.0 do 1.0)
* return: mehanski kot per unit od 0.0 do 1.0 (format IQ)
**************************************************************/
extern float QEP_mehKot(void);

/**************************************************************
* Funkcija, ki vrne vrednost position counterja (QPOSCNT)
* return: vrednost position counterja (QPOSCNT) (format int)
**************************************************************/
extern int QEP_cnt(void);

/**************************************************************
* Funkcija, ki vrne PPR (NUMBER OF PULSES PER POSITION)
* return: pulses per revolution
**************************************************************/
extern long QEP_ppr(void);

/**************************************************************
* Funkcija, ki vrne smer vrtenja
* return: smer vrtenja (format int)
**************************************************************/
extern int QEP_dir(void);

#endif  // end of QEP_DRV_H definition


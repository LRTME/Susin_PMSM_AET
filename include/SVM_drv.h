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
#ifndef     __SVM_DRV_H__
#define     __SVM_DRV_H__

#include    "F28x_Project.h"

#include    "define.h"

/* definicije */
// stevilke PWM modulov, ki krmilijo mostic
// ne pozabi spremeniti GPIO mux registrov v "SVM_init()" funkciji
#define     SVM_MODUL1      EPwm1Regs
#define     SVM_MODUL2      EPwm2Regs
#define     SVM_MODUL3      EPwm3Regs

#define		EPWM_FREQ		(CPU_FREQ/2)

// perioda casovnika (v procesorskih cikilh) 
#define     SVM_PERIOD      ((EPWM_FREQ/SWITCH_FREQ) - 1)

// dead time (v nano sekundah)
#define     SVM_DEAD_TIME   400.0

// nacin delovanja casovnika ob emulation stop
// (0-stop at zero, 1-stop immediately, 2-run free)
#define     SVM_DEBUG_MODE  0

// definicije za status mostica
enum SVM_STATE { DISABLE=0, ENABLE, BOOTSTRAP, TRIP };

/**************************************************************
* Inicializacija PWM modula za SVM
* returns: void
**************************************************************/
extern void SVM_init(void);

/**************************************************************
* Pozene casovnike za SVM modul
* returns: void
**************************************************************/
extern void SVM_start(void);

/**************************************************************
* Vklopi vse izhode
* returns: void
**************************************************************/
extern void SVM_enable(void);

/**************************************************************
* Izklopi vse izhode
* returns: void
**************************************************************/
extern void SVM_disable(void);

/**************************************************************
* Izklopi vse tranzistorje - urgentno
* returns: void
**************************************************************/
extern void SVM_trip(void);

/**************************************************************
* Vklopi spodnje 3 tranzistorje
* returns: void
**************************************************************/
extern void SVM_bootstrap(void);

/**************************************************************
* Vklopi zgornje 3 tranzistorje
* returns: void
**************************************************************/
extern void SVM_high(void);

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene periode
* in je natancna na cikel natancno
* returns: void
* arg1:    zelena perioda
**************************************************************/
extern void SVM_period(float perioda);

/**************************************************************
* Funkcija, ki nastavi periodo, za doseganje zeljene frekvence
* in je natancna na cikel natancno
* returns: void
* arg1:    zelena frekvenca
**************************************************************/
extern void SVM_frequency(float frekvenca);

/**************************************************************
* Funkcija, ki nastavi mrtvi cas v nanosekundah
* returns: void
**************************************************************/
extern void SVM_dead_time(float dead_time);

/**************************************************************
* Funkcija, ki na podlagi vklopnega razmerja
* vklopi doloèene tranzistorje
* returns: void
* arg1:    vklopno razmerje [-1.0, +1.0] (IQ format)
**************************************************************/
extern void SVM_update_DC(float duty);

/**************************************************************
* Na podlagi sektor in vklopnega razmerja nastavi PWM module
* returns: void
* arg1:    vklopno razmerje
* arg2:    sektor [1-6]
**************************************************************/
extern void SVM_update_six(float duty, int sektor);

/**************************************************************
* Na podlagi sektor in vklopnega razmerja nastavi PWM module
* returns: void
* arg1:    vklopno razmerje
* arg2:    sektor [1-6]
**************************************************************/
extern void SVM_update_bldc(float duty, int sektor);

/**************************************************************
* Na podlagi Ualpha in Ubeta nastavi PWM module
* returns: void
* arg1:    napetost Ualpha [-1.0, +1.0] (IQ format)
* arg2:    napetost Ubeta [-1.0, +1.0] (IQ format)
**************************************************************/
extern void SVM_update(float Ualpha, float Ubeta);

/**************************************************************
* vrne status (delam/ne delam)
* returns: svm_status
**************************************************************/
extern int  SVM_status(void);

/**************************************************************
* Vrne vklopno razmerje veje
* returns: duty_leg
**************************************************************/
extern float SVM_get_duty_leg1(void);
extern float SVM_get_duty_leg2(void);
extern float SVM_get_duty_leg3(void);

#endif  // SVM_DRV_H

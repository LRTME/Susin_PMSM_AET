/****************************************************************
* FILENAME:     TRIP_overcurrent.c
* DESCRIPTION:  Trip zone funcionality in case of overcurrent
* AUTHOR:       Denis Susin
*
****************************************************************/

#include    "F28x_Project.h"

#include 	"TRIP_overcurrent.h"
#include 	"SVM_drv.h"


/**************************************************************
* Initialize TRIP zone modul for SVM (PWM modul security latch)
* returns: void
**************************************************************/
void TRIP_OC_init(void)
{
	// setting input pin, which triggers trip for PWM
	EALLOW;
	InputXbarRegs.INPUT1SELECT = 22; // GPIO 22 is input for trip zone funcionality
	EDIS;

	/* delay for start routine - allowing hardware protection
	 * to pull-up the voltage on TZ1 pin if neccessary
	 */
	// DELAY_US(1000L);
}

/**************************************************************
* Enable TRIP zone modul for SVM (PWM modul security latch)
* returns: void
**************************************************************/
void TRIP_OC_enable(void)
{
    SVM_MODUL1.TZSEL.bit.OSHT1 = TZ_ENABLE;    		// TZ1 triggers tripzone enabled
    SVM_MODUL2.TZSEL.bit.OSHT1 = TZ_ENABLE;      	// TZ1 triggers tripzone enabled
    SVM_MODUL3.TZSEL.bit.OSHT1 = TZ_ENABLE;      	// TZ1 triggers tripzone enabled
}

/**************************************************************
* Disable TRIP zone modul for SVM (PWM modul security latch)
* returns: void
**************************************************************/
void TRIP_OC_disable(void)
{
    SVM_MODUL1.TZSEL.bit.OSHT1 = TZ_DISABLE;      	// TZ1 triggers tripzone enabled
    SVM_MODUL2.TZSEL.bit.OSHT1 = TZ_DISABLE;      	// TZ1 triggers tripzone enabled
    SVM_MODUL3.TZSEL.bit.OSHT1 = TZ_DISABLE;      	// TZ1 triggers tripzone enabled
}

/**************************************************************
* Clear TRIP zone modul flags for SVM (PWM modul security latch)
* returns: void
**************************************************************/
void TRIP_OC_reset(void)
{
	EALLOW;

	SVM_MODUL1.TZOSTCLR.bit.OST1 = 1;      		// one-shot trip flag is cleared for this EPwmReg
	SVM_MODUL2.TZOSTCLR.bit.OST1 = 1;      		// one-shot  trip flag is cleared for this EPwmReg
	SVM_MODUL3.TZOSTCLR.bit.OST1 = 1;      		// one-shot  trip flag is cleared for this EPwmReg

	SVM_MODUL1.TZCLR.bit.OST = 1;      			// general trip flag is cleared for this EPwmReg
	SVM_MODUL2.TZCLR.bit.OST = 1;      			// general trip flag is cleared for this EPwmReg
	SVM_MODUL3.TZCLR.bit.OST = 1;      			// general trip flag is cleared for this EPwmReg

    EDIS;
}


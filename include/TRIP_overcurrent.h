/****************************************************************
* FILENAME:     TRIP_overcurrent.h
* DESCRIPTION:  Trip zone funcionality in case of overcurrent
* AUTHOR:       Denis Susin
*
****************************************************************/

#ifndef INCLUDE_TRIP_OVERCURRENT_H_
#define INCLUDE_TRIP_OVERCURRENT_H_


/**************************************************************
* Initialize TRIP zone modul for SVM (PWM modul security latch)
* returns: void
**************************************************************/
extern void TRIP_OC_init(void);

/**************************************************************
* Enable TRIP zone modul for SVM (PWM modul security latch)
* returns: void
**************************************************************/
extern void TRIP_OC_enable(void);

/**************************************************************
* Disable TRIP zone modul for SVM (PWM modul security latch)
* returns: void
**************************************************************/
extern void TRIP_OC_disable(void);


#endif /* INCLUDE_TRIP_OVERCURRENT_H_ */

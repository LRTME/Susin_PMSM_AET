/****************************************************************
* FILENAME:     globals.h
* DESCRIPTION:  project wide global variables
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef     __GLOBALS_H__
#define     __GLOBALS_H__

#include    "F28x_Project.h"

#include    "define.h"

// interrupt counter
extern volatile long    interrupt_cnt;
extern volatile long    interrupt_cnt_s;
extern volatile long    interrupt_cnt_min;

// DLOG trigger selection
extern volatile enum TRIGGER {Ref_cnt = 0, Napetost} trigger;

// modulation scheme selection
enum	MODULATION {SVM = 0, SIX_STEP, SINGLE_PHASE_DC};

// control alghorithm selection when SVM modulation scheme is chosen
enum	CONTROL {OPEN_LOOP = 0, CURRENT_CONTROL, SPEED_CONTROL, POSITION_CONTROL};

#endif // end of __GLOBALS_H__ definition

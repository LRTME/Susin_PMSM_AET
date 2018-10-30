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

// DLOG trigger selection
extern volatile enum TRIGGER {Ref_cnt = 0, Napetost} trigger;

#endif // end of __GLOBALS_H__ definition

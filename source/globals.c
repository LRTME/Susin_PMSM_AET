/****************************************************************
* FILENAME:     globals.c
* DESCRIPTION:  project wide global variables
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include "globals.h"

// interrupt counter
volatile long    interrupt_cnt = 0;
volatile long    interrupt_cnt_s = 0;
volatile long    interrupt_cnt_min = 0;

// DLOG trigger selection
extern volatile enum TRIGGER trigger = Ref_cnt;

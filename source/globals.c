/****************************************************************
* FILENAME:     globals.c
* DESCRIPTION:  project wide global variables
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include "globals.h"

// interrupt counter
volatile long    interrupt_cnt = 0;

// DLOG trigger selection
extern volatile enum TRIGGER trigger = Ref_cnt;

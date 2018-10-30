/****************************************************************
* FILENAME:     BACK_loop.h             
* DESCRIPTION:  background code header file
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef     __BACK_LOOP_H__
#define     __BACK_LOOP_H__

#include    "F28x_Project.h"
#include    "define.h"
#include    "globals.h"

#include    "PWM_drv.h"
#include 	"PCB_util.h"

#include	"COMM_handlers.h"
    
/**************************************************************
* Function which executes background loop code
**************************************************************/
extern void BACK_loop(void);
#endif // end of __BACK_LOOP_H__

/****************************************************************
* FILE:         PER_int.h
* DESCRIPTION:  periodic interrupt header file
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef     __PER_INT_H__
#define     __PER_INT_H__

#include    "F28x_Project.h"

#include    "define.h"
#include    "globals.h"

#include    "SVM_drv.h"
#include    "ADC_drv.h"
#include    "PCB_util.h"

#include	"CAP_drv.h"
#include 	"QEP_drv.h"
#include    "SPI_drv.h"

#include    "math.h"

#include    "CLARKE_float.h"
#include    "PARK_float.h"
#include    "IPARK_float.h"

#include	"ABF_omega.h"
#include	"PI_ctrl.h"
#include	"PID_ctrl.h"

#include	"RES_REG.h"
#include	"REP_REG.h"
#include	"DCT_REG.h"
#include	"dual_DCT_REG.h"

#include    "DLOG_gen.h"
#include    "REF_gen.h"

#include    "TRIP_overcurrent.h"


/**************************************************************
* Function, where mechanical measurements is handled:
* - mechanical angle [0.0 1.0] (1.0 means one full mechanical revolution)
* - electrical angle [0.0 1.0] (1.0 means one quarter of revolution, if pole pair is 4)
* - calls function for mechanical speed calculation
* - calls function for mechanical acceleration calculation
**************************************************************/
extern void get_mechanical(void);

/**************************************************************
* Function, where mechanical speed is calculated (out of mechanical angle)
**************************************************************/
extern void get_meh_speed(void);

/**************************************************************
* Function, where mechanical acceleration is calculated (out of mechanical angle)
**************************************************************/
extern void get_meh_accel(void);

/**************************************************************
* Function, where electrical measurements is handled
**************************************************************/
extern void get_electrical(void);

/**************************************************************
* Function, which alignes d axis with phase 1 axis
**************************************************************/
extern void set_null_position(bool reset_procedure);

/**************************************************************
* Function, which checks, if voltages and currents are respecting limits
**************************************************************/
extern void software_protection(void);

/**************************************************************
* Function, which covers control of 3 phase PMSM
**************************************************************/
extern void control_algorithm(void);

/**************************************************************
* Function for open loop control
**************************************************************/
void open_loop_control(void);

/**************************************************************
* Function for current loop control
**************************************************************/
extern void current_loop_control(void);

/**************************************************************
* Function for advanced current loop control,
* where additional current controller in parallel with
* PI controller is added
**************************************************************/
extern void extra_current_loop_control(void);

/**************************************************************
* Function for speed loop control
**************************************************************/
extern void speed_loop_control(void);

/**************************************************************
* Function for position loop control
**************************************************************/
extern void position_loop_control(void);

/**************************************************************
* Function, which resets control alghorithm after trip
**************************************************************/
extern void trip_reset(void);

/**************************************************************
* Function, which clears integral parts and outputs of
* controllers
**************************************************************/
extern void clear_controllers(void);

/**************************************************************
* Function, which clears integral parts and outputs of
* advaced current controllers
**************************************************************/
extern void clear_advanced_controllers(void);

/**************************************************************
* Function, which calculates systems (L filter) phase delay,
* obtained from sweep test and aproximated with a polynomial and
* returns phase delay compensation in degrees [�]
**************************************************************/
float	phase_lag_comp_calc(float phase_leg_freq);

/**************************************************************
* Function which initializes all required for execution of
* interrupt function
**************************************************************/
extern void PER_int_setup(void);

#endif // end of __PER_INT_H__ definition

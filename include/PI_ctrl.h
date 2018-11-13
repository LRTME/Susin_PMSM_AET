/****************************************************************
* FILENAME:     PI_ctrl.h
* DESCRIPTION:  basic PI controller
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef __PI_CTRL_H__
#define __PI_CTRL_H__

typedef struct PI_CTRL_STRUCT
{
    float Ref;            // Input: Reference input 
    float Fdb;            // Input: Feedback input 
    float Err;            // Variable: Error
    float Kp;             // Parameter: Proportional gain
    float Ki;             // Parameter: Integral gain
    float Up;             // Variable: Proportional output 
    float Ui;             // Variable: Integral output 
    float OutMax;         // Parameter: Maximum output 
    float OutMin;         // Parameter: Minimum output
    float Out;            // Output: PID output 
} PI_ctrl;

/*-----------------------------------------------------------------------------
Default initalizer for the PI_ctrl object.
-----------------------------------------------------------------------------*/                     
#define PI_CTRL_DEFAULTS  \
{           \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0     \
}

/*------------------------------------------------------------------------------
 PI function Definition
------------------------------------------------------------------------------*/
extern void PI_ctrl_calc(PI_ctrl *v);

#endif // __PI_CTRL_H__


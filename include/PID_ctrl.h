/****************************************************************
* FILENAME:     PID_ctrl.h
* DESCRIPTION:  full PID controller
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef __PID_CTRL_H__
#define __PID_CTRL_H__

typedef struct PID_CTRL_STRUCT
{
    float Ref;              // Input: Reference input
    float Fdb;              // Input: Feedback input
    float Ff;               // Input: Feedforward input
    float Err;              // Variable: Error
    float Kp;               // Parameter: Proportional gain
    float Ki;               // Parameter: Integral gain = 1/Ti
    float Kd;               // Parameter: Derivative gain
    float Kff;              // Parameter: Feedforward gain
    float Fdc;              // Parameter: Derivative filter cuttoff freqeuncy in Hz
    float Up;               // Variable: Proportional output
    float Ui;               // Variable: Integral output
    float Ud;               // Variable: Derivative output
    float Udf;              // Variable: Filtered derivative output
    float Uff;              // Variable: Feedforward output
    float OutMax;           // Parameter: Maximum output
    float OutMin;           // Parameter: Minimum output
    float Out;              // Output: PID output
    float Fdb1;             // History: Previous Feedback input
    float Sampling_period;  // Parameter: sampling time
} PID_ctrl;

/*-----------------------------------------------------------------------------
Default initalizer for the PID_float object.
-----------------------------------------------------------------------------*/                     
#define PID_CTRL_DEFAULTS  \
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
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0,    \
    0.0     \
}

#define PID_PI2 3.1415926535897932384626433832795

/*------------------------------------------------------------------------------
 PID Macro Definition
------------------------------------------------------------------------------*/
#define PID_CTRL_CALC(v)                                    \
{                                                           \
    v.Err = v.Ref - v.Fdb;                                  \
    v.Up= v.Kp * v.Err;                                     \
    v.Uff= v.Kff * v.Ff;                                    \
    v.Ud = v.Kd * (v.Fdb1 - v.Fdb) / v.Sampling_period;     \
    v.Fdb1 = v.Fdb;                                         \
    v.Udf = v.Ud * ((v.Sampling_period)/(v.Sampling_period + (1.0/(PID_PI2*v.Fdc)))) \
          + v.Udf * ((1.0/(PID_PI2*v.Fdc))/(v.Sampling_period + (1.0/(PID_PI2*v.Fdc))));\
    v.Out = v.Up + v.Ui + v.Udf + v.Uff;                    \
    if (v.Out > v.OutMax)                                   \
    {                                                       \
        v.Out = v.OutMax;                                   \
        if  (v.Ui < 0.0)                                    \
        {                                                   \
            v.Ui = v.Err * v.Sampling_period * v.Ki + v.Ui; \
        }                                                   \
    }                                                       \
    else if (v.Out < v.OutMin)                              \
    {                                                       \
        v.Out = v.OutMin;                                   \
        if  (v.Ui > 0.0)                                    \
        {                                                   \
            v.Ui = v.Err * v.Sampling_period * v.Ki + v.Ui; \
        }                                                   \
    }                                                       \
    else                                                    \
    {                                                       \
        v.Ui = v.Err * v.Sampling_period * v.Ki + v.Ui;     \
    }                                                       \
}    
#endif // __PID_CTRL_H__

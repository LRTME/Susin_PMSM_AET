/****************************************************************
* FILENAME:     PI_ctrl.h
* DESCRIPTION:  basic PI controller
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include "PI_ctrl.h"

/*------------------------------------------------------------------------------
 PI function Definition
------------------------------------------------------------------------------*/
#pragma CODE_SECTION(PI_ctrl_calc, "ramfuncs");
void PI_ctrl_calc(PI_ctrl *v)
{
    // get the error
    v->Err = v->Ref - v->Fdb;
    
    // proportional part
    v->Up = v->Kp * v->Err;
    
    // sum
    v->Out = v->Up + v->Ui;
    
    // integral part (conditionally)
    if (v->Out > v->OutMax)
    {
        v->Out = v->OutMax;
    }
    else if (v->Out < v->OutMin)
    {
        v->Out = v->OutMin;
    }
    else
    {
        v->Ui = v->Ki * v->Err + v->Ui;
    }
}


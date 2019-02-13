/****************************************************************
* FILENAME:     CLARKE_float.h             
* DESCRIPTION:  izracuna inverzno parkovo transformacijo 
* AUTHOR:       Mitja Nemec
* START DATE:   30.3.2013
* VERSION:      1.0
*
****************************************************************/
#ifndef __CLARKE_FLOAT_H__
#define __CLARKE_FLOAT_H__

#include    "math.h"

typedef struct CLARKE_FLOAT_STRUCT
{  
    float  As;  		// Input: phase-a stator variable
	float  Bs;          // Input: phase-b stator variable 
	float  Alpha;		// Output: stationary d-axis stator variable 
	float  Beta;		// Output: stationary q-axis stator variable
} CLARKE_float;	            

/*-----------------------------------------------------------------------------
	Default initalizer for the CLARKE object.
-----------------------------------------------------------------------------*/
#define CLARKE_FLOAT_DEFAULTS   \
{                               \
    0.0,                        \
    0.0,                        \
    0.0,                        \
    0.0                         \
}

/*------------------------------------------------------------------------------
	CLARKE Transformation Macro Definition
------------------------------------------------------------------------------*/
// ze uposteva faktor c = 2/3, tako, da se amplitude pred in po transformaciji ohranjajo
#define CLARKE_FLOAT_CALC(v)						\
{													\
    v.Alpha = v.As;									\
    v.Beta = (v.As + (2*v.Bs)) * 0.57735026918963;  \
}

#endif // __CLARKE_FLOAT_H__


/****************************************************************
* FILENAME:     PARK_float.h             
* DESCRIPTION:  izracuna parkovo transformacijo 
* AUTHOR:       Mitja Nemec
* START DATE:   30.3.2013
* VERSION:      1.0
*
****************************************************************/
#ifndef     __PARK_FLOAT_H__
#define     __PARK_FLOAT_H__

#include    "math.h"

#ifndef		PI
#define     PI              3.1415926535897932384626433832795
#endif

typedef struct PARK_FLOAT_STRUCT
{  
    float  Alpha;       // Input: stationary d-axis stator variable 
	float  Beta;	 	// Input: stationary q-axis stator variable 
	float  Angle;		// Input: rotating angle (pu) 
	float  Ds;		    // Output: rotating d-axis stator variable 
	float  Qs;		    // Output: rotating q-axis stator variable
} PARK_float;	            

/*-----------------------------------------------------------------------------
Default initalizer for the PARK object.
-----------------------------------------------------------------------------*/                   
#define PARK_FLOAT_DEFAULTS \
{                           \
    0.0,                    \
    0.0,                    \
    0.0,                    \
    0.0,                    \
    0.0                     \
}

/*------------------------------------------------------------------------------
	PARK Transformation Macro Definition
------------------------------------------------------------------------------*/
#define PARK_FLOAT_CALC(v)										\
{																\
	v.Ds = (v.Alpha * cos(2*PI*v.Angle)) + (v.Beta * sin(2*PI*v.Angle));  \
    v.Qs = (v.Beta * cos(2*PI*v.Angle)) - (v.Alpha * sin(2*PI*v.Angle));  \
}

#endif // __PARK_FLOAT_H__

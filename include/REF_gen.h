/****************************************************************
* FILENAME:     REF_gen.h
* DESCRIPTION:  generator tazliènih oblik signala
* AUTHOR:       Mitja Nemec
*
****************************************************************/

#ifndef __INCLUDE_REF_GEN__
#define __INCLUDE_REF_GEN__

#include    "define.h"
#include    "math.h"

// slew rate for step and Konst type
#define REF_GEN_MAX_SLEW    10000.0

/**************************************************************
* calculate new sample of the reference signal
**************************************************************/
void REF_GEN_update(void);

// typdef for signal type
enum REF_GEN_TYPE { REF_Konst, REF_Step, REF_Slew, REF_Sine};

// reference generator struct
struct REF_GEN
{
    float   amp;
    float   freq;
    float   angle;
    float   duty;
    float   offset;
    float   slew;
    float   samp_period;
    float   out;
    float   n_out;
    enum REF_GEN_TYPE type;
};

extern struct REF_GEN ref_gen;

#endif /* INCLUDE_REF_GEN_H_ */

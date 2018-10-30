/****************************************************************
* FILENAME:     REF_gen.c
* DESCRIPTION:  reference generator
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include    "REF_gen.h"

struct REF_GEN ref_gen =
{
        0.0,            //float   ref_amp;
        1.0,            //float   ref_freq;
        0.0,            //float   ref_angle;
        0.5,            //float   ref_duty;
        0.0,            //float   ref_offset;
        1.0,            //float   ref_slew;
        SAMPLE_TIME,    //float   ref_samp_period;
        0.0,
        0.0,
        REF_Sine
};

// private slew rate limit function
void   ref_gen_slew_limit(float input, float *output, float slew_rate);

/**************************************************************
* calculate new sample of the reference signal
**************************************************************/
#pragma CODE_SECTION(REF_GEN_update, "ramfuncs");
void REF_GEN_update(void)
{
    // temporary output
    float   ref_internal = 0.0;

    // generate angle
    ref_gen.angle = ref_gen.angle + ref_gen.freq * ref_gen.samp_period;
    if (ref_gen.angle > 1.0)
    {
        ref_gen.angle = ref_gen.angle - 1.0;
    }
    if (ref_gen.angle < 0.0)
    {
        ref_gen.angle = ref_gen.angle + 1.0;
    }

    // choose which type to generate
    switch(ref_gen.type)
    {
    case REF_Step:
        if (ref_gen.angle < ref_gen.duty)
        {
            ref_internal = ref_gen.amp + ref_gen.offset;
        }
        else
        {
            ref_internal = ref_gen.offset;
        }
        // even the step signal has limited slew rate
        ref_gen_slew_limit(ref_internal, &ref_gen.out, REF_GEN_MAX_SLEW);

        break;
    case REF_Slew:
        if (ref_gen.angle < ref_gen.duty)
        {
            ref_internal = ref_gen.amp + ref_gen.offset;
        }
        else
        {
            ref_internal = ref_gen.offset;
        }
        // limit slew rate
        ref_gen_slew_limit(ref_internal, &ref_gen.out, ref_gen.slew);
        break;

    case REF_Konst:
        // limit slew rate
        ref_gen_slew_limit(ref_gen.amp, &ref_gen.out, REF_GEN_MAX_SLEW);
        break;

    case REF_Sine:
        ref_gen.out = ref_gen.offset + ref_gen.amp * cos(2*PI*ref_gen.angle);
        ref_gen.n_out = ref_gen.offset + ref_gen.amp * sin(2*PI*ref_gen.angle);
        break;

    default:
        ref_gen.out = 0.0;
        break;

    }
}

/**************************************************************
* limit slew rate of the signal
**************************************************************/
void   ref_gen_slew_limit(float input, float *output, float slew_rate)
{
    if ((*output - input) < 0.0)
    {
        if ((-(*output - input)) > slew_rate * ref_gen.samp_period)
            *output = *output + slew_rate * ref_gen.samp_period;
        else
            *output = input;
    }
    if ((*output - input) > 0.0)
    {
        if ((*output - input) > slew_rate * ref_gen.samp_period)
            *output = *output - slew_rate * ref_gen.samp_period;
        else
            *output = input;
    }
}

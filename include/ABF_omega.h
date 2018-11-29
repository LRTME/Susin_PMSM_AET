/****************************************************************
* FILENAME:     ABF_omega.h
* DESCRIPTION:  Alfa-Beta filter za izracun hitrosti iz kota
* AUTHOR:       David Kavreèiè, Mitja Nemec
****************************************************************/
#ifndef     __ABF_OMEGA__
#define     __ABF_OMEGA__

#define     ABF_OMEGA_SAMPLING_FREQ     20000

// Diskretizacijski korak trackerja
#define     ABF_OMEGA_SAMPLING_PERIOD   (1.0 / ABF_OMEGA_SAMPLING_FREQ)


typedef struct ABF_OMEGA_STRUCT
{
	float KotIn;        // Izmerjen kot
	float KotOut;       // Ocenjen  kot
	float Omega;        // Ocenjena hitrost
	float Error;        // Napaka med dejanskim in izmerjenim kotom
	float Alpha;        // Parametra trackerja
	float Beta;
} ABF_omega;

#define ABF_OMEGA_DEFAULTS  \
{                           \
	0.0,                    \
	0.0,                    \
	0.0,                    \
	0.0,                    \
	0.0,                    \
    0.0                     \
}

#define ABF_OMEGA_CALC(v)                                                   \
{                                                                           \
    v.KotOut = v.KotOut + (v.Omega * ABF_OMEGA_SAMPLING_PERIOD);            \
    if (v.KotOut > 1.0)                                                     \
    {                                                                       \
        v.KotOut = v.KotOut - 1.0;                                          \
    }                                                                       \
    else if (v.KotOut < 0.0)                                                \
    {                                                                       \
        v.KotOut = v.KotOut + 1.0;                                          \
    }                                                                       \
    v.Error = v.KotIn - v.KotOut;                                        	\
    if (v.Error > +0.5)                                                     \
    {                                                                       \
        v.Error = v.Error - 1.0;                                            \
    }                                                                       \
    else if (v.Error < -0.5)                                                \
    {                                                                       \
        v.Error = v.Error + 1.0;                                            \
    }                                                                       \
    v.KotOut = v.KotOut + (v.Alpha * v.Error);                              \
    v.Omega = v.Omega + ((v.Beta  * v.Error) * ABF_OMEGA_SAMPLING_FREQ);  	\
}                                                                           \

#endif  // __ABF_OMEGA__

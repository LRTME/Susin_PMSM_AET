/****************************************************************
* FILENAME:     define.h           
* DESCRIPTION:  file with global define macros
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef     __DEFINE_H__
#define     __DEFINE_H__

#include	"stddef.h"
#include	"stdint.h"

// switching frequency
#define     SWITCH_FREQ     20000L

// ratio between switching and sampling frequency
#define     SAMP_PRESCALE   1

// sampling frequency
#define     SAMPLE_FREQ     (SWITCH_FREQ/SAMP_PRESCALE)

// sampling period
#define     SAMPLE_TIME     (1.0/SAMPLE_FREQ)

// CPU speed
#define     CPU_FREQ        200000000L

// math constants
#define     SQRT3           1.7320508075688772935274463415059
#define     SQRT2           1.4142135623730950488016887242097
#define     PI              3.1415926535897932384626433832795

// bool type definition
typedef enum {FALSE = 0, TRUE} bool;

// how peripherals behave on debug event
// 0 stop immediately, 1 stop when finished, 2 run free
#define     DEBUG_STOP      0





/* nazivni podatki PMSM-ja */

// nazivna napetost in norma [V]
#define     NORMA_U         24.0
// nazivni tok in norma [A]
#define     NORMA_I         40.0
//  impedanca norma [Ohm]
#define     NORMA_Z         NORMA_U/NORMA_I
// nazivni navor in norma [Nm]
#define     NORMA_M         5.89
// nazivna vrtilna hitrost in norma [vrt/min]
#define     NORMA_n         900
// norma mehanskega kota [vrt]
#define     NORMA_KOT_MEH   1.0


/* parametri PMSM-ja */

// polovi pari [1]
#define     POLE_PAIRS    	4
// upornost statorskega navitja [Ohm]
#define     Rs              46.9e-3
// statorska induktivnost v vzdolzni smeri [H]
#define     Ld              24e-6
// statorska induktivnost v preèni smeri [H]
#define     Lq              56e-6
// mag. pretok trajnih magnetov [Wb]
#define     PSI_ROT         0.019630984202985
// vztrajnostni moment motorja [kgm^2]
#define     J               0.004740530992912




#endif // end of __DEFINE_H__ definition

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

// parametri tokovnih regulatorjev

/*

                    TOKOVNI REGULATOR
datum       |   parametri delujoèih regulatorjev    |     vrednost     |         komentar
---------------------------------------------------------------------------------------------
29.11.2018  |   P del regulatorja toka v d smeri    |      0.0311      |   I. parametriranje na 12 V
29.11.2018  |   I del regulatorja toka v d smeri    | 26.0/SAMPLE_FREQ |   I. parametriranje na 12 V

29.11.2018  |   P del regulatorja toka v q smeri    |      0.0311      |   I. parametriranje na 12 V
29.11.2018  |   I del regulatorja toka v q smeri    | 26.0/SAMPLE_FREQ |   I. parametriranje na 12 V
---------------------------------------------------------------------------------------------
29.11.2018  |   P del regulatorja toka v d smeri    |      0.0156      |   I. parametriranje na 24 V
29.11.2018  |   I del regulatorja toka v d smeri    | 13.0/SAMPLE_FREQ |   I. parametriranje na 24 V

29.11.2018  |   P del regulatorja toka v q smeri    |      0.0156      |   I. parametriranje na 24 V
29.11.2018  |   I del regulatorja toka v q smeri    | 13.0/SAMPLE_FREQ |   I. parametriranje na 24 V
---------------------------------------------------------------------------------------------




// parametri hitrostnih regulatorjev (s 4 razliènimi naèini merjenja hitrosti)

RDIFF:
                    HITROSTNI REGULATOR
datum       |   parametri delujoèih regulatorjev    |  vrednost |           komentar
---------------------------------------------------------------------------------------------
29.11.2018  |   P del regulatorja hitrosti          |   2.00    |   DC link: 24 V, merjenje hitrosti: RDIFF velja za buffer: 201
29.11.2018  |   I del regulatorja hitrosti          |   2e-4    |   DC link: 24 V, merjenje hitrosti: RDIFF velja za buffer: 201
29.11.2018  |   D del regulatorja hitrosti          |   0.00    |   DC link: 24 V, merjenje hitrosti: RDIFF velja za buffer: 201

HOLOBRODKO:
                    HITROSTNI REGULATOR
datum       |   parametri delujoèih regulatorjev    |  vrednost |           komentar
---------------------------------------------------------------------------------------------
29.11.2018  |   P del regulatorja hitrosti          |   1.00    |   DC link: 24 V, merjenje hitrosti: HOLOBRODKO velja za buffer: 101 in N_vzorcev: 11
29.11.2018  |   I del regulatorja hitrosti          |   5e-5    |   DC link: 24 V, merjenje hitrosti: HOLOBRODKO velja za buffer: 101 in N_vzorcev: 11
29.11.2018  |   D del regulatorja hitrosti          |   0.00    |   DC link: 24 V, merjenje hitrosti: HOLOBRODKO velja za buffer: 101 in N_vzorcev: 11

                    HITROSTNI REGULATOR
datum       |   parametri delujoèih regulatorjev    |  vrednost |           komentar
---------------------------------------------------------------------------------------------
29.11.2018  |   P del regulatorja hitrosti          |   1.50    |   DC link: 24 V, merjenje hitrosti: HOLOBRODKO velja za buffer: 201 in N_vzorcev: 11
29.11.2018  |   I del regulatorja hitrosti          |   1e-4    |   DC link: 24 V, merjenje hitrosti: HOLOBRODKO velja za buffer: 201 in N_vzorcev: 11
29.11.2018  |   D del regulatorja hitrosti          |   0.00    |   DC link: 24 V, merjenje hitrosti: HOLOBRODKO velja za buffer: 201 in N_vzorcev: 11

CAP:
                    HITROSTNI REGULATOR
datum       |   parametri delujoèih regulatorjev    |  vrednost |           komentar
---------------------------------------------------------------------------------------------
29.11.2018  |   P del regulatorja hitrosti          |   5.00    |   DC link: 24 V, merjenje hitrosti: CAP velja za prescaler: 2 in qualification: 0 - preveè "navit"
29.11.2018  |   I del regulatorja hitrosti          |   1e-3    |   DC link: 24 V, merjenje hitrosti: CAP velja za prescaler: 2 in qualification: 0 - preveè "navit"
29.11.2018  |   D del regulatorja hitrosti          |   0.00    |   DC link: 24 V, merjenje hitrosti: CAP velja za prescaler: 2 in qualification: 0

                    HITROSTNI REGULATOR
datum       |   parametri delujoèih regulatorjev    |  vrednost |           komentar
---------------------------------------------------------------------------------------------
29.11.2018  |   P del regulatorja hitrosti          |   3.00    |   DC link: 24 V, merjenje hitrosti: CAP velja za prescaler: 2 in qualification: 0 - preveè "navit"
29.11.2018  |   I del regulatorja hitrosti          |   5e-4    |   DC link: 24 V, merjenje hitrosti: CAP velja za prescaler: 2 in qualification: 0 - preveè "navit"
29.11.2018  |   D del regulatorja hitrosti          |   0.00    |   DC link: 24 V, merjenje hitrosti: CAP velja za prescaler: 2 in qualification: 0

ABF:
                    HITROSTNI REGULATOR
datum       |   parametri delujoèih regulatorjev    |  vrednost |           komentar
---------------------------------------------------------------------------------------------
29.11.2018  |   P del regulatorja hitrosti          |   0.45    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 800 Hz
29.11.2018  |   I del regulatorja hitrosti          |   5e-5    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 800 Hz
29.11.2018  |   D del regulatorja hitrosti          |   0.00    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 800 Hz

                    HITROSTNI REGULATOR
datum       |   parametri delujoèih regulatorjev    |  vrednost |           komentar
---------------------------------------------------------------------------------------------
5.12.2018   |   P del regulatorja hitrosti          |   1.50    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz - mehko delovanje
5.12.2018   |   I del regulatorja hitrosti          |   1e-4    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz - mehko delovanje
5.12.2018   |   D del regulatorja hitrosti          |   0.00    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz - mehko delovanje

                    HITROSTNI REGULATOR
datum       |   parametri delujoèih regulatorjev    |  vrednost |           komentar
---------------------------------------------------------------------------------------------
5.12.2018   |   P del regulatorja hitrosti          |   3.00    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz - agresivno delovanje
5.12.2018   |   I del regulatorja hitrosti          |   5e-4    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz - agresivno delovanje
5.12.2018   |   D del regulatorja hitrosti          |   0.00    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz - agresivno delovanje




// parametri regulatorjev pri regulaciji pozicije

                    HITROSTNI REGULATOR
datum       |   parametri delujoèih regulatorjev    |  vrednost |           komentar
---------------------------------------------------------------------------------------------
5.12.2018   |   P del regulatorja hitrosti          |   5.00    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz
5.12.2018   |   I del regulatorja hitrosti          |   1e-3    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz
5.12.2018   |   D del regulatorja hitrosti          |   0.00    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz


                    POZICIJSKI REGULATOR
datum       |   parametri delujoèih regulatorjev    |  vrednost |           komentar
---------------------------------------------------------------------------------------------
5.12.2018   |   P del regulatorja hitrosti          |   50.0    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz
5.12.2018   |   I del regulatorja hitrosti          |   0.00    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz
5.12.2018   |   D del regulatorja hitrosti          |   1e-3    |   DC link: 24 V, merjenje hitrosti: ABF velja za dusenje: sqrt(2)/2 in mejno_frekvenco: 100 Hz



*/

#endif // end of __DEFINE_H__ definition

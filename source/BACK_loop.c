/****************************************************************
 * FILENAME:     BACK_loop.c
 * DESCRIPTION:  background code
 * AUTHOR:       Mitja Nemec
 *
 ****************************************************************/

#include "BACK_loop.h"

// module wide global variables
const char test_string[] = "this is a test string";
const char str_entered_backloop[] = "entered background loop";

bool 	pulse_10ms = FALSE;
bool 	pulse_100ms = FALSE;
bool 	pulse_500ms = FALSE;
bool 	pulse_1000ms = FALSE;
int  	cnt_limit = 5;

bool 	sw1_state = FALSE;
bool 	b1_state = FALSE;
bool 	b2_state = FALSE;
bool 	b3_state = FALSE;
bool 	b4_state = FALSE;

bool 	sw1_press = FALSE;
bool 	b1_press = FALSE;
bool 	b2_press = FALSE;
bool 	b3_press = FALSE;
bool 	b4_press = FALSE;
bool 	b1_press_int = FALSE;
bool 	b2_press_int = FALSE;
bool 	b3_press_int = FALSE;
bool 	b4_press_int = FALSE;

// declaration of external variables


// declaration of function prototypes
void pulse_gen(void);
void scan_keys(void);

/**************************************************************
* Function which executes background loop code
**************************************************************/
#pragma CODE_SECTION(BACK_loop, "ramfuncs");
void BACK_loop(void)
{
    // local variables

    // signal entereing backgorund loop
    COMM_send_string(str_entered_backloop, sizeof(str_entered_backloop));

    // send startup configuration
    COMM_send_initial();

    // infinite background loop
    while (1)
    {
        // pulse generator
        pulse_gen();

        // scan pressed keys
        scan_keys();

        // communication stack
        COMM_runtime();
/*
        // zelena LED utripa s frekvenco 1 Hz in s tem signalizira pravilno delovanje
        if(pulse_500ms == TRUE)
        {
        	PCB_LED2_toggle();
        }
*/
        // toggling LED with button press
/*
        if (b1_press == TRUE)
        {
            PCB_LED1_toggle();
        }
        if (b2_press == TRUE)
        {
            PCB_LED2_toggle();
        }
        if (b3_press == TRUE)
        {
            PCB_LED3_toggle();
        }
        if (b4_press == TRUE)
        {
            PCB_LED4_toggle();
        }
*/

        asm(" NOP");
    }   // end of while(1)
}       // end of BACK_loop

void pulse_gen(void)
{
    static long interrupt_cnt_old_10ms = 0;
    static long interrupt_cnt_old_100ms = 0;
    static int  pulse_500ms_cnt = 0;
    static int  pulse_1000ms_cnt = 0;

    long delta_cnt_10ms;
    long delta_cnt_100ms;

    if ( (interrupt_cnt - interrupt_cnt_old_10ms) < 0)
    {
        interrupt_cnt_old_10ms = interrupt_cnt_old_10ms - (SAMPLE_FREQ);
    }

    if ( (interrupt_cnt - interrupt_cnt_old_100ms) < 0)
    {
        interrupt_cnt_old_100ms = interrupt_cnt_old_100ms - (SAMPLE_FREQ);
    }


    delta_cnt_10ms = interrupt_cnt - interrupt_cnt_old_10ms;
    delta_cnt_100ms = interrupt_cnt - interrupt_cnt_old_100ms;


    // generiraj pulz vsakih 10 ms
    if (delta_cnt_10ms > ((SAMPLE_FREQ)/100))
    {
        pulse_10ms = TRUE;
        interrupt_cnt_old_10ms = interrupt_cnt;
    }
    else
    {
        pulse_10ms = FALSE;
    }

    // generiraj pulz vsakih 100 ms
    if (delta_cnt_100ms > ((SAMPLE_FREQ)/10))
    {
        pulse_100ms = TRUE;
        interrupt_cnt_old_100ms = interrupt_cnt;
    }
    else
    {
        pulse_100ms = FALSE;
    }

    // stejem pulze po 100ms, da dobim pulz 500 ss
    if (pulse_100ms == 1)
    {
        pulse_500ms_cnt = pulse_500ms_cnt + 1;
        if (pulse_500ms_cnt == 5)
        {
            pulse_500ms = TRUE;
            pulse_500ms_cnt = 0;
        }
        else
        {
            pulse_500ms = FALSE;
        }
    }
    // da pulz traja samo in samo eno iteracijo
    if ((pulse_100ms != 1) && (pulse_500ms == 1))
    {
        pulse_500ms = FALSE;
    }

    // stejem pulze po 100ms, da dobim pulz 1 s
    if (pulse_100ms == 1)
    {
        pulse_1000ms_cnt = pulse_1000ms_cnt + 1;
        if (pulse_1000ms_cnt == 10)
        {
            pulse_1000ms = TRUE;
            pulse_1000ms_cnt = 0;
        }
        else
        {
            pulse_1000ms = FALSE;
        }
    }
    // da pulz traja samo in samo eno iteracijo
    if ((pulse_100ms != 1) && (pulse_1000ms == 1))
    {
        pulse_1000ms = FALSE;
    }
}

#pragma CODE_SECTION(scan_keys, "ramfuncs");
void scan_keys(void)
{
    // lokalne spremenljivke
    static int sw1_cnt = 0;
    static int b1_cnt = 0;
    static int b2_cnt = 0;
    static int b3_cnt = 0;
    static int b4_cnt = 0;

    // scan every fixed time interval
    if (pulse_10ms == 1)
    {
        // preberem trenutna stanja tipk
        sw1_state = PCB_SW1_read();
        b1_state = PCB_B1_read();
        b2_state = PCB_B2_read();
        b3_state = PCB_B3_read();
        b4_state = PCB_B4_read();

        // ali smo pritisnili na stikalo 1
        // ce je stikalo pritisnjeno, stopaj koliko casa je pritisnjeno
        if (sw1_state == TRUE)
        {
            sw1_cnt = sw1_cnt + 1;
        }
        // ce ni pritisnjena resetiraj stevec
        else
        {
            sw1_cnt = 0;
        }

        // ce je tipka pritisnjena dovolj casa, javi programu - samo enkrat
        if (sw1_cnt == cnt_limit)
        {
            sw1_press = TRUE;
        }
        // sicer pa ne javi
        else
        {
            sw1_press = FALSE;
        }

        // ali smo pritisnili na tipko 1
        // ce je tipka pritisnjena, stopaj koliko casa je pritisnjena
        if (b1_state == TRUE)
        {
            b1_cnt = b1_cnt + 1;
        }
        // ce ni pritisnjena resetiraj stevec
        else
        {
            b1_cnt = 0;
        }

        // ce je tipka pritisnjena dovolj casa, javi programu - samo enkrat
        if (b1_cnt == cnt_limit)
        {
            b1_press = TRUE;
            b1_press_int = TRUE;
        }
        // sicer pa ne javi
        else
        {
            b1_press = FALSE;
            b1_press_int = FALSE;
        }

        // ali smo pritisnili na tipko 2
		// ce je tipka pritisnjena, stopaj koliko casa je pritisnjena
		if (b2_state == TRUE)
		{
			b2_cnt = b2_cnt + 1;
		}
		// ce ni pritisnjena resetiraj stevec
		else
		{
			b2_cnt = 0;
		}

		// ce je tipka pritisnjena dovolj casa, javi programu - samo enkrat
		if (b2_cnt == cnt_limit)
		{
			b2_press = TRUE;
			b2_press_int = TRUE;
		}
		// sicer pa ne javi
		else
		{
			b2_press = FALSE;
			b2_press_int = FALSE;
		}

        // ali smo pritisnili na tipko 3
		// ce je tipka pritisnjena, stopaj koliko casa je pritisnjena
		if (b3_state == TRUE)
		{
			b3_cnt = b3_cnt + 1;
		}
		// ce ni pritisnjena resetiraj stevec
		else
		{
			b3_cnt = 0;
		}

		// ce je tipka pritisnjena dovolj casa, javi programu - samo enkrat
		if (b3_cnt == cnt_limit)
		{
			b3_press = TRUE;
			b3_press_int = TRUE;
		}
		// sicer pa ne javi
		else
		{
			b3_press = FALSE;
			b3_press_int = FALSE;
		}

        // ali smo pritisnili na tipko 4
		// ce je tipka pritisnjena, stopaj koliko casa je pritisnjena
		if (b4_state == TRUE)
		{
			b4_cnt = b4_cnt + 1;
		}
		// ce ni pritisnjena resetiraj stevec
		else
		{
			b4_cnt = 0;
		}

		// ce je tipka pritisnjena dovolj casa, javi programu - samo enkrat
		if (b4_cnt == cnt_limit)
		{
			b4_press = TRUE;
			b4_press_int = TRUE;
		}
		// sicer pa ne javi
		else
		{
			b4_press = FALSE;
			b4_press_int = FALSE;
		}
    }
    // da je pulz dolg res samo in samo eno iteracijo
    else
    {
        sw1_press = FALSE;
        b1_press = FALSE;
        b2_press = FALSE;
        b3_press = FALSE;
        b4_press = FALSE;
    }
}

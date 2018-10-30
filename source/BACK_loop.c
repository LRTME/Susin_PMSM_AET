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

bool pulse_1000ms = FALSE;
bool pulse_500ms = FALSE;
bool pulse_100ms = FALSE;
bool pulse_50ms = FALSE;
bool pulse_10ms = FALSE;

// declaration of external variables


// declaration of function prototypes
void pulse_gen(void);

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

        // communication stack
        COMM_runtime();

        // once a second (when interrupt_cnt roles over) toggled LEDs on PCB
        if (pulse_1000ms == TRUE)
        {
            PCB_LEDpad1_toggle();
            PCB_LEDpad2_toggle();
            // demonstration how to send a string
            COMM_send_string(test_string, sizeof(test_string));
        }

        asm(" NOP");
    }   // end of while(1)
}       // end of BACK_loop

void pulse_gen(void)
{
    static long interrupt_cnt_old_500ms = 0;
    static long interrupt_cnt_old_100ms = 0;
    static long interrupt_cnt_old_50ms = 0;
    static long interrupt_cnt_old_10ms = 0;
    static int  pulse_1000ms_cnt = 0;

    long delta_cnt_500ms;
    long delta_cnt_100ms;
    long delta_cnt_50ms;
    long delta_cnt_10ms;

    if ( (interrupt_cnt - interrupt_cnt_old_500ms) < 0)
    {
        interrupt_cnt_old_500ms = interrupt_cnt_old_500ms - (SAMPLE_FREQ);
    }

    if ( (interrupt_cnt - interrupt_cnt_old_100ms) < 0)
    {
        interrupt_cnt_old_100ms = interrupt_cnt_old_100ms - (SAMPLE_FREQ);
    }

    if ( (interrupt_cnt - interrupt_cnt_old_50ms) < 0)
    {
        interrupt_cnt_old_50ms = interrupt_cnt_old_50ms - (SAMPLE_FREQ);
    }

    if ( (interrupt_cnt - interrupt_cnt_old_10ms) < 0)
    {
        interrupt_cnt_old_10ms = interrupt_cnt_old_10ms - (SAMPLE_FREQ);
    }

    delta_cnt_500ms = interrupt_cnt - interrupt_cnt_old_500ms;
    delta_cnt_100ms = interrupt_cnt - interrupt_cnt_old_100ms;
    delta_cnt_50ms = interrupt_cnt - interrupt_cnt_old_50ms;
    delta_cnt_10ms = interrupt_cnt - interrupt_cnt_old_10ms;

    // generiraj pulza vsakih 0.01
    if (delta_cnt_10ms > ((SAMPLE_FREQ)/100))
    {
        pulse_10ms = TRUE;
        interrupt_cnt_old_10ms = interrupt_cnt;
    }
    else
    {
        pulse_10ms = FALSE;
    }

    // generiraj pulze vsakih 0.05
    if (delta_cnt_50ms > ((SAMPLE_FREQ)/50))
    {
        pulse_50ms = TRUE;
        interrupt_cnt_old_50ms = interrupt_cnt;
    }
    else
    {
        pulse_50ms = FALSE;
    }


    // generiraj pulza vsakih 0.1
    if (delta_cnt_100ms > ((SAMPLE_FREQ)/10))
    {
        pulse_100ms = TRUE;
        interrupt_cnt_old_100ms = interrupt_cnt;
    }
    else
    {
        pulse_100ms = FALSE;
    }

    // generiraj pulza vsakih 0.5
    if (delta_cnt_500ms > ((SAMPLE_FREQ)/2))
    {
        pulse_500ms = TRUE;
        interrupt_cnt_old_500ms = interrupt_cnt;
    }
    else
    {
        pulse_500ms = FALSE;
    }

    // stejem pulze po 100ms
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
    if ((pulse_100ms != 1)&&(pulse_1000ms == 1))
    {
        pulse_1000ms = FALSE;
    }
}

/****************************************************************
* FILENAME:     DLOG_gen.c
* DESCRIPTION:  generic data logger module
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include "DLOG_gen.h"

// buffer data
DLOG_GEN_TYPE DLOG_b_1[DLOG_GEN_SIZE];
#if DLOG_GEN_NR > 1
DLOG_GEN_TYPE DLOG_b_2[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 2
DLOG_GEN_TYPE DLOG_b_3[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 3
DLOG_GEN_TYPE DLOG_b_4[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 4
DLOG_GEN_TYPE DLOG_b_5[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 5
DLOG_GEN_TYPE DLOG_b_6[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 6
DLOG_GEN_TYPE DLOG_b_7[DLOG_GEN_SIZE];
#endif
#if DLOG_GEN_NR > 7
DLOG_GEN_TYPE DLOG_b_8[DLOG_GEN_SIZE];
#endif

struct DLOG dlog = 
{
    0UL,
    0UL,
    #if DLOG_GEN_NR > 1
    0UL,
    #endif
    #if DLOG_GEN_NR > 2
    0UL,
    #endif
    #if DLOG_GEN_NR > 3
    0UL,
    #endif
    #if DLOG_GEN_NR > 4
    0UL,
    #endif
    #if DLOG_GEN_NR > 5
    0UL,
    #endif
    #if DLOG_GEN_NR > 6
    0UL,
    #endif
    #if DLOG_GEN_NR > 7
    0UL,
    #endif

    // common data
    0UL,
    1,
    0,
    0,
    DLOG_GEN_SIZE,
    Wait,
    Positive,
    DLOG_GEN_SIZE,
    0L,
    DLOG_GEN_SIZE,
    0L,
    Auto,
    0
};

/**************************************************************
* update DLOG module
**************************************************************/
#pragma CODE_SECTION(DLOG_GEN_update, "ramfuncs");
void DLOG_GEN_update(void)
{
    // in continuous mode, just store the date
    if (dlog.mode == Continuous)
    {
        // if this sample is right then store
        if (dlog.skip_cntr ==0)
        {
            DLOG_b_1[dlog.write_ptr] = *(dlog.iptr1);

            #if DLOG_GEN_NR > 1
            DLOG_b_2[dlog.write_ptr] = *(dlog.iptr2);
            #endif

            #if DLOG_GEN_NR > 2
            DLOG_b_3[dlog.write_ptr] = *(dlog.iptr3);
            #endif

            #if DLOG_GEN_NR > 3
            DLOG_b_4[dlog.write_ptr] = *(dlog.iptr4);
            #endif

            #if DLOG_GEN_NR > 4
            DLOG_b_5[dlog.write_ptr] = *(dlog.iptr5);
            #endif

            #if DLOG_GEN_NR > 5
            DLOG_b_6[dlog.write_ptr] = *(dlog.iptr6);
            #endif

            #if DLOG_GEN_NR > 6
            DLOG_b_7[dlog.write_ptr] = *(dlog.iptr7);
            #endif

            #if DLOG_GEN_NR > 7
            DLOG_b_8[dlog.write_ptr] = *(dlog.iptr8);
            #endif

            (dlog.write_ptr)++;
            // wrap the pointer around if necessary
            if (dlog.write_ptr == dlog.write_length)
            {
                dlog.write_ptr = 0;
            }
        }
        // otherwise downsample for prescalar amount
        (dlog.skip_cntr)++;
        if ((dlog.skip_cntr) >= (dlog.downsample_ratio))
        {
            dlog.skip_cntr = 0;
        }
    }
    else
    {
    // if waiting for trigger pre-condition
    if ((dlog.state == Wait))
    {
        // if stopped wait for mode change or SW trigger
        if (dlog.mode == Stop)
        {
            if (dlog.sw_trigger != 0)
            {
                dlog.state = Ready;
                dlog.mode = Single;
                dlog.sw_trigger = 0;
            }
        }
        // check for trigger condition or auto trigger
        else
        {
            // Auto trigger in Auto mode
            if (dlog.mode == Auto)
            {
                dlog.auto_cnt = dlog.auto_cnt + 1;
                if (dlog.auto_cnt == dlog.auto_time)
                {
                    dlog.state = Store;
                    dlog.auto_cnt = 0;
                }
            }
            // ready trigger if positive slope (value is below trigg value)
            if (dlog.slope == Positive)
            {
                if(*(dlog.trig) < dlog.trig_level) dlog.state = Ready;
            }
            // ready trigger if negative slope (value is above trigg value)
            else
            {
                if(*(dlog.trig) > dlog.trig_level) dlog.state = Ready;
            }
            // ce je sw triger potem kar sproži
            if (dlog.sw_trigger != 0)
            {
                dlog.state = Ready;
                dlog.sw_trigger = 0;
            }
        }
    }
    // wait for actual trigger condition
    if (dlog.state == Ready)
    {
        // Auto trigger in Auto mode
        if (dlog.mode == Auto)
        {
            dlog.auto_cnt = dlog.auto_cnt + 1;
            if (dlog.auto_cnt == dlog.auto_time)
            {
                dlog.state = Store;
                dlog.auto_cnt = 0;
            }
        }
        //  Check for positive slope trigger event
        if (dlog.slope == Positive)
        {
            if(*(dlog.trig) >= dlog.trig_level)
            {
                dlog.state = Store;
                dlog.auto_cnt = 0;
            }
        }
        // check for negative slope trigger event 
        else
        {
            if(*(dlog.trig) <= dlog.trig_level)
            {
                dlog.state = Store;
                dlog.auto_cnt = 0;
            }

        }

        // or SW trigger
        if (dlog.sw_trigger != 0)
        {
            dlog.state = Store;
            dlog.auto_cnt = 0;
            dlog.sw_trigger = 0;
        }
    }

    // if in store mode store until full
    if (dlog.state == Store)
    {
        // if this sample is right then store
        if (dlog.skip_cntr ==0)
        {
            DLOG_b_1[dlog.write_ptr] = *(dlog.iptr1);

            #if DLOG_GEN_NR > 1
            DLOG_b_2[dlog.write_ptr] = *(dlog.iptr2);
            #endif

            #if DLOG_GEN_NR > 2
            DLOG_b_3[dlog.write_ptr] = *(dlog.iptr3);
            #endif

            #if DLOG_GEN_NR > 3
            DLOG_b_4[dlog.write_ptr] = *(dlog.iptr4);
            #endif

            #if DLOG_GEN_NR > 4
            DLOG_b_5[dlog.write_ptr] = *(dlog.iptr5);
            #endif

            #if DLOG_GEN_NR > 5
            DLOG_b_6[dlog.write_ptr] = *(dlog.iptr6);
            #endif

            #if DLOG_GEN_NR > 6
            DLOG_b_7[dlog.write_ptr] = *(dlog.iptr7);
            #endif

            #if DLOG_GEN_NR > 7
            DLOG_b_8[dlog.write_ptr] = *(dlog.iptr8);
            #endif

            (dlog.write_ptr)++;

            // when full stop and prepera for next trigger
            if (dlog.write_ptr == dlog.write_length)
            {
                dlog.write_ptr = 0;
                dlog.skip_cntr = 0;

                if (dlog.mode != Single)
                {
                    dlog.state = Holdoff;
                    dlog.holdoff_cnt = 0;
                }
                else
                {
                    dlog.mode = Stop;
                    dlog.state = Wait;
                }
            }
        }
        
        // otherwise downsample for prescalar amount
        (dlog.skip_cntr)++;
        if ((dlog.skip_cntr) >= (dlog.downsample_ratio))
        {
            dlog.skip_cntr = 0;
        }
    }

    // wait for holdoff time before lokking for new trigger
    if (dlog.state == Holdoff)
    {
        dlog.holdoff_cnt = dlog.holdoff_cnt + 1;
        if (dlog.holdoff_cnt == dlog.holdoff_time)
        {
            dlog.state = Wait;
        }
    }
    }
}

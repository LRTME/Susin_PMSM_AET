/****************************************************************
* FILENAME:     TIC_toc.h             
* DESCRIPTION:  Timer 1 as a stopwatch
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef     __TIC_TOC_H__
#define     __TIC_TOC_H__

volatile long TIC_time = 0;

/*------------------------------------------------------------------------------
TIC & TOC macros
------------------------------------------------------------------------------*/
#define TIC_init()                          \
{                                           \
    CpuTimer0Regs.PRD.all  = 0xFFFFFFFF;    \
    CpuTimer0Regs.TPR.all  = 0;             \
    CpuTimer0Regs.TPRH.all = 0;             \
    CpuTimer0Regs.TCR.bit.TSS = 1;          \
    CpuTimer0Regs.TCR.bit.TRB = 1;          \
}


#define TIC_start()                     \
{                                       \
    CpuTimer0Regs.TCR.bit.TSS = 0;      \
    TIC_time = CpuTimer0Regs.TIM.all;   \
}
        
#define TIC_stop()                      \
{                                       \
    CpuTimer0Regs.TCR.bit.TSS = 1;      \
    TIC_time = TIC_time                 \
             - CpuTimer0Regs.TIM.all;   \
    CpuTimer0Regs.PRD.all  = 0xFFFFFFFF;    \
}

#define TIC_laptime   (CpuTimer0Regs.TIM.all)
    

        
#endif // __TIC_TOC_H__

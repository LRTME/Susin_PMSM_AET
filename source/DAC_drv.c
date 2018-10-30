/**************************************************************
* FILE:         DAC_drv.c
* DESCRIPTION:  DAC driver for 28377
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#include "DAC_drv.h"

/**************************************************************
* Initialize DAC
**************************************************************/
void DAC_init(bool use_external_ref)
{
    // Enable DACOUTA
    EALLOW;

    //Use VDAC as the reference for DAC
    if (use_external_ref == TRUE)
    {
        DacaRegs.DACCTL.bit.DACREFSEL  = 0;
        DacbRegs.DACCTL.bit.DACREFSEL  = 0;
    }
    // othwerwise use internal reference
    else
    {
        DacaRegs.DACCTL.bit.DACREFSEL  = 1;
        DacbRegs.DACCTL.bit.DACREFSEL  = 1;
    }

    // Enable DAC output
    DacaRegs.DACOUTEN.bit.DACOUTEN = 1;
    DacbRegs.DACOUTEN.bit.DACOUTEN = 1;

    EDIS;

}

/**************************************************************
* Update channel
* returns:
* arg1: voltage_pu - Per Unit reference oltage [0.0, 1.0]
**************************************************************/
#pragma CODE_SECTION(DAC_update_a, "ramfuncs");
void DAC_update_a(float voltage_pu)
{
    int code;

    code = voltage_pu * 4096;

    if (code >= 4096)
    {
        code = 4095;
    }
    if (code < 0)
    {
        code = 0;
    }
    
    DacaRegs.DACVALS.bit.DACVALS = code;
}

/**************************************************************
* Update channel
* returns:
* arg1: voltage_pu - Per Unit reference oltage [0.0, 1.0]
**************************************************************/
#pragma CODE_SECTION(DAC_update_b, "ramfuncs");
void DAC_update_b(float voltage_pu)
{
    int code;

    code = voltage_pu * 4096;

    if (code >= 4096)
    {
        code = 4095;
    }
    if (code < 0)
    {
        code = 0;
    }

    DacbRegs.DACVALS.bit.DACVALS = code;
}

/**************************************************************
* Update channel
* returns:
* arg1: voltage_pu - Per Unit reference oltage [-1.0, +1.0]
**************************************************************/
#pragma CODE_SECTION(DAC_update_a_signed, "ramfuncs");
void DAC_update_a_signed(float voltage_pu)
{
    int code;

    code = voltage_pu * 2048 + 2048;

    // preverim meje
    if (code > 4096)
    {
        code = 4095;
    }
    if (code < 0.0)
    {
        code = 0;
    }

    DacaRegs.DACVALS.bit.DACVALS = code;
}

/**************************************************************
* Update channel
* returns:
* arg1: voltage_pu - Per Unit reference oltage [-1.0, +1.0]
**************************************************************/
#pragma CODE_SECTION(DAC_update_b_signed, "ramfuncs");
void DAC_update_b_signed(float voltage_pu)
{
    int code;

    code = voltage_pu * 2048 + 2048;

    // preverim meje
    if (code > 4096)
    {
        code = 4095;
    }
    if (code < 0)
    {
        code = 0;
    }

    DacbRegs.DACVALS.bit.DACVALS = code;
}

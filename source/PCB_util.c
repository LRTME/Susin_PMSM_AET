/**************************************************************
* FILE:         PCB_util.c 
* DESCRIPTION:  PCB initialization & Support Functions
* AUTHOR:       Mitja Nemec
*
**************************************************************/
#include "PCB_util.h"

/**************************************************************
* turn the LED1 on Launchpad on
***************************************************************/
#pragma CODE_SECTION(PCB_LEDpad1_on, "ramfuncs");
void PCB_LEDpad1_on(void)
{
    GpioDataRegs.GPBSET.bit.GPIO34 = 1;
}

/**************************************************************
* turn the LED1 on Launchpad off
***************************************************************/
#pragma CODE_SECTION(PCB_LEDpad1_off, "ramfuncs");
void PCB_LEDpad1_off(void)
{
    GpioDataRegs.GPBSET.bit.GPIO34 = 1;
}

/**************************************************************
* toggle the LED1 on Launchpad
***************************************************************/
#pragma CODE_SECTION(PCB_LEDpad1_toggle, "ramfuncs");
void PCB_LEDpad1_toggle(void)
{
    GpioDataRegs.GPBTOGGLE.bit.GPIO34 = 1;
}

/**************************************************************
* turn the LED2 on Launchpad on
***************************************************************/
#pragma CODE_SECTION(PCB_LEDpad2_on, "ramfuncs");
void PCB_LEDpad2_on(void)
{
    GpioDataRegs.GPASET.bit.GPIO31 = 1;
}

/**************************************************************
* turn the LED2 on Launchpad off
***************************************************************/
#pragma CODE_SECTION(PCB_LEDpad2_off, "ramfuncs");
void PCB_LEDpad2_off(void)
{
    GpioDataRegs.GPACLEAR.bit.GPIO31 = 1;
}

/**************************************************************
* toggle turn the LED1 on Launchpad
***************************************************************/
#pragma CODE_SECTION(PCB_LEDpad2_toggle, "ramfuncs");
void PCB_LEDpad2_toggle(void)
{
    GpioDataRegs.GPATOGGLE.bit.GPIO31 = 1;
}

/**************************************************************
* initialize specific GPIO functions
**************************************************************/
void PCB_init(void)
{
    /* OUTPUTS */
    // LED na LaunchPadu
    GPIO_SetupPinMux(34, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(34, GPIO_OUTPUT, GPIO_PUSHPULL);

    GPIO_SetupPinMux(31, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(31, GPIO_OUTPUT, GPIO_PUSHPULL);

}

/**************************************************************
* FILE:         PCB_util.c 
* DESCRIPTION:  PCB initialization & Support Functions
* AUTHOR:       Mitja Nemec
* CHANGES:
* 				Denis Sušin    5.11.2018    Inputs and outputs on a 3 phase inverter PCB with LAUNCHPAD TMSF28379D
**************************************************************/
#include "PCB_util.h"

/**************************************************************
* Initialize specific GPIO functions
**************************************************************/
void PCB_init(void)
{
    /* INPUTS */
    // 4 buttons and a switch

	// Button 1
	GPIO_SetupPinMux(139, GPIO_MUX_CPU1, 0);
	GPIO_SetupPinOptions(139, GPIO_INPUT, GPIO_INPUT);

	// Button 2
	GPIO_SetupPinMux(56, GPIO_MUX_CPU1, 0);
	GPIO_SetupPinOptions(56, GPIO_INPUT, GPIO_INPUT);

	// Button 3
	GPIO_SetupPinMux(97, GPIO_MUX_CPU1, 0);
	GPIO_SetupPinOptions(97, GPIO_INPUT, GPIO_INPUT);

	// Button 4
	GPIO_SetupPinMux(94, GPIO_MUX_CPU1, 0);
	GPIO_SetupPinOptions(94, GPIO_INPUT, GPIO_INPUT);

	// Switch 1
	GPIO_SetupPinMux(32, GPIO_MUX_CPU1, 0);
	GPIO_SetupPinOptions(32, GPIO_INPUT, GPIO_INPUT);




    /* OUTPUTS and their default states */
    // 4 LEDs

	// LED 1
    GPIO_SetupPinMux(65, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(65, GPIO_OUTPUT, GPIO_OUTPUT);
    GPIO_WritePin(65,LED_OFF);

    // LED 2
    GPIO_SetupPinMux(52, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(52, GPIO_OUTPUT, GPIO_OUTPUT);
    GPIO_WritePin(52,LED_ON);

    // LED 3
    GPIO_SetupPinMux(41, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(41, GPIO_OUTPUT, GPIO_OUTPUT);
    GPIO_WritePin(41,LED_OFF);

    // LED 4
    GPIO_SetupPinMux(40, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(40, GPIO_OUTPUT, GPIO_OUTPUT);
    GPIO_WritePin(40,LED_OFF);
}


/**************************************************************
* Function, that returns state of the switch
**************************************************************/
bool PCB_SW1_read(void)
{
	if (GpioDataRegs.GPBDAT.bit.GPIO32 == 1)
	{
		return (FALSE);
	}
	else
	{
		return (TRUE);
	}
}

/**************************************************************
* Function, that returns state of the button 1
**************************************************************/
bool PCB_B1_read(void)
{
	if (GpioDataRegs.GPEDAT.bit.GPIO139 == 1)
	{
		return (FALSE);
	}
	else
	{
		return (TRUE);
	}
}

/**************************************************************
* Function, that returns state of the button 2
**************************************************************/
bool PCB_B2_read(void)
{
	if (GpioDataRegs.GPBDAT.bit.GPIO56 == 1)
	{
		return (FALSE);
	}
	else
	{
		return (TRUE);
	}
}

/**************************************************************
* Function, that returns state of the button 3
**************************************************************/
bool PCB_B3_read(void)
{
	if (GpioDataRegs.GPDDAT.bit.GPIO97 == 1)
	{
		return (FALSE);
	}
	else
	{
		return (TRUE);
	}
}

/**************************************************************
* Function, that returns state of the button 4
**************************************************************/
bool PCB_B4_read(void)
{
	if (GpioDataRegs.GPCDAT.bit.GPIO94 == 1)
	{
		return (FALSE);
	}
	else
	{
		return (TRUE);
	}
}

/**************************************************************
* Function, that returns state of the overcurrent trip input
**************************************************************/
bool PCB_TRIP_OC_read(void)
{
	if (GpioDataRegs.GPADAT.bit.GPIO22 == 1)
	{
		return (FALSE);
	}
	else
	{
		return (TRUE);
	}
}

/**************************************************************
* turn the LED1 on Launchpad on
***************************************************************/
#pragma CODE_SECTION(PCB_LED1_on, "ramfuncs");
void PCB_LED1_on(void)
{
    GpioDataRegs.GPCSET.bit.GPIO65 = 1;
}

/**************************************************************
* turn the LED1 on Launchpad off
***************************************************************/
#pragma CODE_SECTION(PCB_LED1_off, "ramfuncs");
void PCB_LED1_off(void)
{
    GpioDataRegs.GPCSET.bit.GPIO65 = 1;
}

/**************************************************************
* toggle the LED1 on Launchpad
***************************************************************/
#pragma CODE_SECTION(PCB_LED1_toggle, "ramfuncs");
void PCB_LED1_toggle(void)
{
    GpioDataRegs.GPCTOGGLE.bit.GPIO65 = 1;
}


/**************************************************************
* turn the LED2 on Launchpad on
***************************************************************/
#pragma CODE_SECTION(PCB_LED2_on, "ramfuncs");
void PCB_LED2_on(void)
{
    GpioDataRegs.GPBSET.bit.GPIO52 = 1;
}

/**************************************************************
* turn the LED2 on Launchpad off
***************************************************************/
#pragma CODE_SECTION(PCB_LED2_off, "ramfuncs");
void PCB_LED2_off(void)
{
    GpioDataRegs.GPBSET.bit.GPIO52 = 1;
}

/**************************************************************
* toggle the LED2 on Launchpad
***************************************************************/
#pragma CODE_SECTION(PCB_LED2_toggle, "ramfuncs");
void PCB_LED2_toggle(void)
{
    GpioDataRegs.GPBTOGGLE.bit.GPIO52 = 1;
}


/**************************************************************
* turn the LED3 on Launchpad on
***************************************************************/
#pragma CODE_SECTION(PCB_LED3_on, "ramfuncs");
void PCB_LED3_on(void)
{
    GpioDataRegs.GPBSET.bit.GPIO41 = 1;
}

/**************************************************************
* turn the LED3 on Launchpad off
***************************************************************/
#pragma CODE_SECTION(PCB_LED3_off, "ramfuncs");
void PCB_LED3_off(void)
{
    GpioDataRegs.GPBSET.bit.GPIO41 = 1;
}

/**************************************************************
* toggle the LED3 on Launchpad
***************************************************************/
#pragma CODE_SECTION(PCB_LED3_toggle, "ramfuncs");
void PCB_LED3_toggle(void)
{
    GpioDataRegs.GPBTOGGLE.bit.GPIO41 = 1;
}


/**************************************************************
* turn the LED4 on Launchpad on
***************************************************************/
#pragma CODE_SECTION(PCB_LED4_on, "ramfuncs");
void PCB_LED4_on(void)
{
    GpioDataRegs.GPBSET.bit.GPIO40 = 1;
}

/**************************************************************
* turn the LED4 on Launchpad off
***************************************************************/
#pragma CODE_SECTION(PCB_LED4_off, "ramfuncs");
void PCB_LED4_off(void)
{
    GpioDataRegs.GPBSET.bit.GPIO40 = 1;
}

/**************************************************************
* toggle the LED4 on Launchpad
***************************************************************/
#pragma CODE_SECTION(PCB_LED4_toggle, "ramfuncs");
void PCB_LED4_toggle(void)
{
    GpioDataRegs.GPBTOGGLE.bit.GPIO40 = 1;
}

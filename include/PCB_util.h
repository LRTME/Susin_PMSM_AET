/**************************************************************
* FILE:         PCB_util.h 
* DESCRIPTION:  definitions for PCB Initialization & Support Functions
* AUTHOR:       Mitja Nemec
*
**************************************************************/
#ifndef   PCB_UTIL_H
#define   PCB_UTIL_H

#include    "F28x_Project.h"
#include    "define.h"

/**************************************************************
* turn the LED1 on Launchpad on
***************************************************************/
extern void PCB_LEDpad1_on(void);

/**************************************************************
* turn the LED1 on Launchpad of
***************************************************************/
extern void PCB_LEDpad1_off(void);

/**************************************************************
* toggle the LED1 on Launchpad
***************************************************************/
extern void PCB_LEDpad1_toggle(void);

/**************************************************************
* turn the LED2 on Launchpad on
***************************************************************/
extern void PCB_LEDpad2_on(void);

/**************************************************************
* turn the LED2 on Launchpad off
***************************************************************/
extern void PCB_LEDpad2_off(void);

/**************************************************************
* toggle the LED2 on Launchpad
***************************************************************/
extern void PCB_LEDpad2_toggle(void);

/**************************************************************
* initialize specific GPIO functions
**************************************************************/
extern void PCB_init(void);


#endif  // end of PCB_UTIL_H definition


/**************************************************************
* FILE:         DAC_drv.c
* DESCRIPTION:  DAC driver for 28377
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef     __DAC_DRV_H__
#define     __DAC_DRV_H__

#include    "F28x_Project.h"
#include    "define.h"

/**************************************************************
* Initialize DAC
* returns:
* arg1: use_external_ref - bool to indicate whether to use external reference
**************************************************************/
extern void DAC_init(bool use_external_ref);

/**************************************************************
* Update channel
* returns:
* arg1: voltage_pu - Per Unit reference oltage [0.0, 1.0]
**************************************************************/
extern void DAC_update_a(float voltage_pu);

/**************************************************************
* Update channel
* returns:
* arg1: voltage_pu - Per Unit reference oltage [0.0, 1.0]
**************************************************************/
extern void DAC_update_b(float voltage_pu);

/**************************************************************
* Update channel
* returns:
* arg1: voltage_pu - Per Unit reference oltage [-1.0, +1.0]
**************************************************************/
extern void DAC_update_a_signed(float voltage_pu);

/**************************************************************
* Update channel
* returns:
* arg1: voltage_pu - Per Unit reference oltage [-1.0, +1.0]
**************************************************************/
extern void DAC_update_b_signed(float voltage_pu);

#endif /* __ADC_DRV_H__ */

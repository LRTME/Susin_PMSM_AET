/**************************************************************
* FILE:         SPI_dajalnik.h 
* DESCRIPTION:  definitions for SPI_dajalnik Initialization & Support Functions
* VERSION:      1.0
* AUTHOR:       Mitja Nemec
**************************************************************/
#ifndef   __SPI_DRV_H__
#define   __SPI_DRV_H__

// kateri SPI modul uporabljamo
#define     SPI_MODUL   SpiaRegs

/**************************************************************
* Funkcija, ki inicializira komunikacijo
**************************************************************/
extern void SPI_init(int bits, long clock, int clk_phase, int clk_polarity);

/**************************************************************
* Funkcija, ki komunicira z SPI dajalnikom položaja
* returns:  koda, ki jo vrne dajalnik
**************************************************************/
extern unsigned int SPI_getset_blocking(unsigned int data_out);

#endif  // end of __SPI_DRV_H__ definition


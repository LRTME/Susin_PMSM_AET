/****************************************************************
* FILENAME:     CRC_mitja.h             
* DESCRIPTION:  declarations of 16bit CRC for MODBUS protocol Functions.  
* AUTHOR:       Mitja Nemec, Thomas Pircher
*
****************************************************************/

#ifndef   __CRC_MITJA_H__
#define   __CRC_MITJA_H__


/**************************************************************
* Initialization function
**************************************************************/
extern void CRC_init(void);

/**************************************************************
* Function which calculates partial CRC from a single byte
* (lower eight bits of an integer)
**************************************************************/
extern void CRC_add_byte(int byte);

/**************************************************************
* function which returns calculated CRC
**************************************************************/
extern int CRC_get(void);

#endif  // __CRC_MITJA_H__


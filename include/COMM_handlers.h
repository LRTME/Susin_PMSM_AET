/****************************************************************
* FILENAME:     COMM_handlers.h
* DESCRIPTION:  Communication handlers
* AUTHOR:       Mitja Nemec
*
****************************************************************/

#ifndef     __INCLUDE_COMM_HANDLERS_H__
#define     __INCLUDE_COMM_HANDLERS_H__

#include    "F28x_Project.h"
#include    "define.h"
#include    "globals.h"

#include    "DLOG_gen.h"
#include    "REF_gen.h"

#include    "LRTME_stack.h"

#include    "PCB_util.h"

// baud rate
#define     COMM_BAUDRATE       (250000L)

/**************************************************************
* Function which initializes complete communication stack
* returns:
**************************************************************/
extern void COMM_initialization(void);
extern void COMM_send_initial(void);
/**************************************************************
* Function which periodicaly sends required packets (DLOG, ...)
* and checks for new packets
* returns:
**************************************************************/
extern void COMM_runtime(void);

/**************************************************************
* Function which sends ASCII string.
* Should only be called from BACK_loop context
* returns:
**************************************************************/
extern void COMM_send_string(const char *string, unsigned int length);

extern void COMM_send_status(void);

#endif /* INCLUDE_COMM_HANDLERS_H_ */

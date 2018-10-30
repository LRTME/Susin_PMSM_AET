/****************************************************************
* FILENAME:     LRTME_stack.h
* DESCRIPTION:  LRTME stack code
* AUTHOR:       Mitja Nemec
*
****************************************************************/
#ifndef     __LETMR_STACK_H__
#define     __LETMR_STACK_H__

#include    "F28x_Project.h"
#include    "define.h"

#include    "COBS_drv.h"
#include    "SCI_drv.h"

// transmission packet length
#define     LRTME_TX_BUFF_SIZE      2048

// transmission queue length - number of packest to send at once
#define     LRTME_TX_QUEUE_SIZE     20

// receive packet length
#define     LRTME_RX_BUFF_SIZE      40

// number of recevice callback functions
#define     LRTME_RX_NR_HANDLERS    40

/**************************************************************
* LRTME stack initialization
**************************************************************/
extern void LRTME_init(void);

/**************************************************************
* LRTME stack main function
**************************************************************/
extern void LRTME_stack(void);

/**************************************************************
* LRTME are we in quiet mode?
**************************************************************/
extern bool LRTME_quiet(void);

/**************************************************************
* return the number of elements on a send queue
**************************************************************/
extern int LRTME_tx_queue_poll(void);

/**************************************************************
* put a new packet on send queue
**************************************************************/
extern void LRTME_send(int code, int *data, int length, void    (*tx_callback)(void));

/**************************************************************
* function which registeres receive callback function
**************************************************************/
extern void LRTME_receive_register(int code, void (*function)(int *));

#endif // end of __LETMR_STACK_H__
